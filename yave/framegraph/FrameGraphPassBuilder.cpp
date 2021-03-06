/*******************************
Copyright (c) 2016-2019 Gr�goire Angerand

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

#include "FrameGraphPassBuilder.h"
#include "FrameGraph.h"

namespace yave {

FrameGraphPassBuilder::FrameGraphPassBuilder(FrameGraphPass* pass) : _pass(pass) {
}

void FrameGraphPassBuilder::set_render_func(FrameGraphPass::render_func&& func) {
	_pass->_render = std::move(func);
}


// --------------------------------- Declarations ---------------------------------

FrameGraphMutableImageId FrameGraphPassBuilder::declare_image(ImageFormat format, const math::Vec2ui& size) {
	return parent()->declare_image(format, size);
}

FrameGraphMutableBufferId FrameGraphPassBuilder::declare_buffer(usize byte_size) {
	return parent()->declare_buffer(byte_size);
}

FrameGraphMutableImageId FrameGraphPassBuilder::declare_copy(FrameGraphImageId src) {
	const auto& src_info = parent()->info(src);
	auto res = declare_image(src_info.format, src_info.size);
	_pass->_image_copies.emplace_back(src, res);
	// copies are done before the pass proper so we don't set the stage
	add_to_pass(src, ImageUsage::TransferSrcBit, PipelineStage::None);
	add_to_pass(res, ImageUsage::TransferDstBit, PipelineStage::None);
	return res;
}


// --------------------------------- Texture ---------------------------------

void FrameGraphPassBuilder::add_texture_input(FrameGraphImageId res, PipelineStage stage) {
	add_to_pass(res, ImageUsage::TextureBit, stage);
}


// --------------------------------- Framebuffer ---------------------------------

void FrameGraphPassBuilder::add_depth_output(FrameGraphMutableImageId res, Framebuffer::LoadOp load_op) {
	// transition is done by the renderpass
	add_to_pass(res, ImageUsage::DepthBit, PipelineStage::DepthAttachmentOutBit);
	if(_pass->_depth.image.is_valid()) {
		y_fatal("Pass already has a depth output.");
	}
	_pass->_depth = {res, load_op};
}

void FrameGraphPassBuilder::add_color_output(FrameGraphMutableImageId res, Framebuffer::LoadOp load_op) {
	// transition is done by the renderpass
	add_to_pass(res, ImageUsage::ColorBit, PipelineStage::ColorAttachmentOutBit);
	_pass->_colors << FrameGraphPass::Attachment{res, load_op};
}


// --------------------------------- Copies ---------------------------------

void FrameGraphPassBuilder::add_copy_src(FrameGraphImageId res) {
	add_to_pass(res, ImageUsage::TransferSrcBit, PipelineStage::TransferBit);
}

void FrameGraphPassBuilder::add_copy_dst(FrameGraphMutableImageId res) {
	add_to_pass(res, ImageUsage::TransferDstBit, PipelineStage::TransferBit);
}

void FrameGraphPassBuilder::add_copy_src(FrameGraphBufferId res) {
	add_to_pass(res, BufferUsage::TransferSrcBit, PipelineStage::TransferBit);
}

void FrameGraphPassBuilder::add_copy_dst(FrameGraphMutableBufferId res) {
	add_to_pass(res, BufferUsage::TransferDstBit, PipelineStage::TransferBit);
}


// --------------------------------- Storage output ---------------------------------

void FrameGraphPassBuilder::add_storage_output(FrameGraphMutableImageId res, usize ds_index, PipelineStage stage) {
	add_to_pass(res, ImageUsage::StorageBit, stage);
	add_uniform(FrameGraphDescriptorBinding::create_storage_binding(res), ds_index);
}

void FrameGraphPassBuilder::add_storage_output(FrameGraphMutableBufferId res, usize ds_index, PipelineStage stage) {
	add_to_pass(res, BufferUsage::StorageBit, stage);
	add_uniform(FrameGraphDescriptorBinding::create_storage_binding(res), ds_index);
}


// --------------------------------- Storage intput ---------------------------------

void FrameGraphPassBuilder::add_storage_input(FrameGraphBufferId res, usize ds_index, PipelineStage stage) {
	add_to_pass(res, BufferUsage::StorageBit, stage);
	add_uniform(FrameGraphDescriptorBinding::create_storage_binding(res), ds_index);
}

void FrameGraphPassBuilder::add_storage_input(FrameGraphImageId res, usize ds_index, PipelineStage stage) {
	add_to_pass(res, ImageUsage::StorageBit, stage);
	add_uniform(FrameGraphDescriptorBinding::create_storage_binding(res), ds_index);
}


// --------------------------------- Uniform input ---------------------------------

void FrameGraphPassBuilder::add_uniform_input(FrameGraphBufferId res, usize ds_index, PipelineStage stage) {
	add_to_pass(res, BufferUsage::UniformBit, stage);
	add_uniform(FrameGraphDescriptorBinding::create_uniform_binding(res), ds_index);
}

void FrameGraphPassBuilder::add_uniform_input(FrameGraphImageId res, usize ds_index, PipelineStage stage) {
	add_to_pass(res, ImageUsage::TextureBit, stage);
	add_uniform(FrameGraphDescriptorBinding::create_uniform_binding(res), ds_index);
}


// --------------------------------- External ---------------------------------

Y_TODO(external framegraph resources are not synchronized)
void FrameGraphPassBuilder::add_uniform_input(StorageView tex, usize ds_index, PipelineStage) {
	add_uniform(Binding(tex), ds_index);
}

void FrameGraphPassBuilder::add_uniform_input(TextureView tex, usize ds_index, PipelineStage) {
	add_uniform(Binding(tex), ds_index);
}

void FrameGraphPassBuilder::add_uniform_input(CubemapView tex, usize ds_index, PipelineStage) {
	add_uniform(Binding(tex), ds_index);
}


// --------------------------------- Attribs ---------------------------------

void FrameGraphPassBuilder::add_attrib_input(FrameGraphBufferId res, PipelineStage stage) {
	add_to_pass(res, BufferUsage::AttributeBit, stage);
}

void FrameGraphPassBuilder::add_index_input(FrameGraphBufferId res, PipelineStage stage) {
	add_to_pass(res, BufferUsage::IndexBit, stage);
}


// --------------------------------- stuff ---------------------------------

void FrameGraphPassBuilder::add_descriptor_binding(Binding bind, usize ds_index) {
	add_uniform(bind, ds_index);
}

template<typename T>
void set_stage(T& info, PipelineStage stage) {
	if(info.stage != PipelineStage::None) {
		Y_TODO(This should be either one write or many reads)
		y_fatal("Resource can only be used once per pass (previous stage was %).", usize(info.stage));
	}
	info.stage = stage;
}

void FrameGraphPassBuilder::add_to_pass(FrameGraphImageId res, ImageUsage usage, PipelineStage stage) {
	res.check_valid();
	auto& info = _pass->_images[res];
	set_stage(info, stage);
	parent()->register_usage(res, usage, _pass);
}

void FrameGraphPassBuilder::add_to_pass(FrameGraphBufferId res, BufferUsage usage, PipelineStage stage) {
	res.check_valid();
	auto& info = _pass->_buffers[res];
	set_stage(info, stage);
	parent()->register_usage(res, usage, _pass);
}

void FrameGraphPassBuilder::add_uniform(FrameGraphDescriptorBinding binding, usize ds_index) {
	auto& bindings = _pass->_bindings;
	while(bindings.size() <= ds_index) {
		bindings.emplace_back();
	}
	bindings[ds_index].push_back(binding);
}

void FrameGraphPassBuilder::set_cpu_visible(FrameGraphMutableBufferId res) {
	parent()->set_cpu_visible(res, _pass);
}

FrameGraph* FrameGraphPassBuilder::parent() {
	return _pass->_parent;
}

}
