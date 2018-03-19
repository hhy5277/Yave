/*******************************
Copyright (c) 2016-2018 Gr�goire Angerand

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

#include "EngineView.h"

#include <yave/renderers/TiledDeferredRenderer.h>
#include <yave/renderers/ToneMapper.h>

#include <y/io/File.h>

#include <imgui/imgui.h>

namespace editor {

EngineView::EngineView(DevicePtr dptr) :
		DeviceLinked(dptr),
		_uniform_buffer(device(), 1) {
}

void EngineView::set_scene_view(NotOwner<SceneView*> scene_view) {
	if(_scene_view == scene_view) {
		return;
	}

	_scene_view = scene_view;
	_renderer = nullptr;
}


void EngineView::set_render_size(math::Vec2ui size) {
	static constexpr u32 min_size(256);
	for(auto& i : size) {
		i = std::max(i, min_size);
	}

	if(size != _size) {
		_size = size;
		create_renderer();
	}
}

void EngineView::create_renderer() {
	if(!_scene_view) {
		_renderer = nullptr;
		return;
	}

	log_msg(core::str() + "creating with size = " + _size.x() + ", " + _size.y());

	auto scene		= Node::Ptr<SceneRenderer>(new SceneRenderer(device(), *_scene_view));
	auto gbuffer	= Node::Ptr<GBufferRenderer>(new GBufferRenderer(scene, _size));
	auto deferred	= Node::Ptr<TiledDeferredRenderer>(new TiledDeferredRenderer(gbuffer));
	auto tonemap	= Node::Ptr<SecondaryRenderer>(new ToneMapper(deferred));

	_renderer		= Node::Ptr<FramebufferRenderer>(new FramebufferRenderer(tonemap, _size));

	_ui_material = Material(device(), MaterialData()
			.set_frag_data(SpirVData::from_file(io::File::open("engine_view.frag.spv").expected("Unable to load spirv file.")))
			.set_vert_data(SpirVData::from_file(io::File::open("screen.vert.spv").expected("Unable to load spirv file.")))
			.set_bindings({Binding(_renderer->output()), Binding(_uniform_buffer)})
			.set_depth_tested(false)
		);
}

void EngineView::render(CmdBufferRecorder<>& recorder, const FrameToken& token) {
	if(!_renderer) {
		return;
	}

	{
		RenderingPipeline pipeline(_renderer);
		pipeline.render(recorder, token);
	}
	// so we don't have to wait when resizing
	recorder.keep_alive(_renderer);
}

void EngineView::paint_ui() {
	if(ImGui::BeginDock("Engine view")) {
		bool render = _renderer;

		math::Vec2 size = ImGui::GetWindowSize();
		math::Vec2 pos = ImGui::GetWindowPos();
		set_render_size(math::Vec2ui(size));

		if(render) {
			auto map = TypedMapping(_uniform_buffer);
			map[0] = ViewData{math::Vec2i(size), math::Vec2i(pos), math::Vec2i(_size)};

			ImGui::GetWindowDrawList()->AddCallback(reinterpret_cast<ImDrawCallback>(&draw_callback), this);
		} else {
			ImGui::Text("Error, unable to setup renderer...");
		}
	}

	ImGui::EndDock();
}

void EngineView::draw_callback(RenderPassRecorder& recorder, void* user_data) {
	reinterpret_cast<EngineView*>(user_data)->render_ui(recorder);
}

void EngineView::render_ui(RenderPassRecorder& recorder) {
	if(!_renderer) {
		fatal("Internal error.");
	}

	auto region = recorder.region("EngineView::render_ui");

	recorder.bind_material(_ui_material);
	recorder.draw(vk::DrawIndirectCommand(6, 1));
}



}