/*******************************
Copyright (c) 2016-2019 Grégoire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/
#include "RenderPass.h"

#include <yave/graphics/images/Image.h>
#include <yave/device/Device.h>

namespace yave {

// image layout inside the render pass (like color attachment optimal)
static vk::ImageLayout vk_initial_image_layout(ImageUsage usage) {
	return vk_image_layout(usage & ImageUsage::Attachment);
}

// image layout outside the renderpass (like shader read optimal)
static vk::ImageLayout vk_final_image_layout(ImageUsage usage) {
	return vk_image_layout(usage);
}

static std::array<vk::SubpassDependency, 2> create_dependencies() {
	return {{vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstAccessMask(
				vk::AccessFlagBits::eColorAttachmentRead |
				vk::AccessFlagBits::eColorAttachmentWrite/* |
				vk::AccessFlagBits::eDepthStencilAttachmentRead |
				vk::AccessFlagBits::eDepthStencilAttachmentWrite*/
		),
		vk::SubpassDependency()
			.setSrcSubpass(0)
			.setDstSubpass(VK_SUBPASS_EXTERNAL)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setSrcAccessMask(
					vk::AccessFlagBits::eColorAttachmentRead |
					vk::AccessFlagBits::eColorAttachmentWrite/* |
					vk::AccessFlagBits::eDepthStencilAttachmentRead |
					vk::AccessFlagBits::eDepthStencilAttachmentWrite*/
				)
			.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
		}};
}

static std::array<vk::SubpassDependency, 2> create_z_pass_dependencies() {
	return {{vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setDstStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstAccessMask(
				vk::AccessFlagBits::eDepthStencilAttachmentRead |
				vk::AccessFlagBits::eDepthStencilAttachmentWrite
		),
		vk::SubpassDependency()
			.setSrcSubpass(0)
			.setDstSubpass(VK_SUBPASS_EXTERNAL)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
			.setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setSrcAccessMask(
					vk::AccessFlagBits::eDepthStencilAttachmentRead |
					vk::AccessFlagBits::eDepthStencilAttachmentWrite
				)
			.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
		}};
}

static vk::AttachmentLoadOp attachment_load_op(const RenderPass::ImageData& image) {
	return image.load_op == RenderPass::LoadOp::Clear
		? vk::AttachmentLoadOp::eClear
		: vk::AttachmentLoadOp::eLoad;
}

static vk::AttachmentDescription create_attachment(RenderPass::ImageData image) {
	auto layout = vk_final_image_layout(image.usage);
	return vk::AttachmentDescription()
		.setFormat(image.format.vk_format())
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(attachment_load_op(image))
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFinalLayout(layout)
		.setInitialLayout(layout)
	;
}

static vk::AttachmentReference create_attachment_reference(ImageUsage usage, usize index) {
	return vk::AttachmentReference()
		.setAttachment(u32(index))
		.setLayout(vk_initial_image_layout(usage))
	;
}


static vk::RenderPass create_renderpass(DevicePtr dptr, RenderPass::ImageData depth, core::ArrayView<RenderPass::ImageData> colors) {
	auto attachments = core::vector_with_capacity<vk::AttachmentDescription>(colors.size() + 1);
	std::transform(colors.begin(), colors.end(), std::back_inserter(attachments), [=](const auto& color) { return create_attachment(color); });

	auto color_refs = core::vector_with_capacity<vk::AttachmentReference>(colors.size());
	for(usize i = 0; i != colors.size(); ++i) {
		color_refs << create_attachment_reference(ImageUsage::ColorBit, i);
	}

	auto subpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(u32(color_refs.size()))
			.setPColorAttachments(color_refs.begin())
		;

	vk::AttachmentReference depth_ref;
	if(depth.usage != ImageUsage::None) {
		attachments << create_attachment(depth);
		depth_ref = create_attachment_reference(ImageUsage::DepthBit, color_refs.size());
		subpass.setPDepthStencilAttachment(&depth_ref);
	}

	auto dependencies = colors.is_empty() ? create_z_pass_dependencies() : create_dependencies();
	return dptr->vk_device().createRenderPass(vk::RenderPassCreateInfo()
			.setAttachmentCount(u32(attachments.size()))
			.setPAttachments(attachments.begin())
			.setSubpassCount(1)
			.setPSubpasses(&subpass)
			.setDependencyCount(dependencies.size())
			.setPDependencies(dependencies.begin())
		)
	;
}

RenderPass::Layout::Layout(ImageData depth, core::ArrayView<ImageData> colors) : _depth(depth.format) {
	_colors.reserve(colors.size());
	std::transform(colors.begin(), colors.end(), std::back_inserter(_colors), [](const auto& e) { return e.format; });
}

u64 RenderPass::Layout::hash() const {
	u64 h = u64(_depth.vk_format());
	for(const auto& c : _colors) {
		hash_combine(h, u64(c.vk_format()));
	}
	return h;
}

bool RenderPass::Layout::is_depth_only() const {
	return _colors.is_empty();
}

bool RenderPass::Layout::operator==(const Layout& other) const {
	return _depth == other._depth && _colors == other._colors;
}


RenderPass::RenderPass(DevicePtr dptr, ImageData depth, core::ArrayView<ImageData> colors) :
		DeviceLinked(dptr),
		_attachment_count(colors.size()),
		_render_pass(create_renderpass(dptr, depth, colors)),
		_layout(depth, colors) {
}

RenderPass::RenderPass(DevicePtr dptr, core::ArrayView<ImageData> colors) :
		RenderPass(dptr, ImageData(), colors) {
}

RenderPass::~RenderPass() {
	destroy(_render_pass);
}

vk::RenderPass RenderPass::vk_render_pass() const {
	return _render_pass;
}

bool RenderPass::is_depth_only() const {
	return layout().is_depth_only();
}

const RenderPass::Layout& RenderPass::layout() const {
	return _layout;
}

usize RenderPass::attachment_count() const {
	return _attachment_count;
}


}
