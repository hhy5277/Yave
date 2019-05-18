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
#include "EntityView.h"

#include <editor/context/EditorContext.h>

#include <yave/objects/Light.h>

#include <imgui/imgui_yave.h>

// remove:
#include <yave/material/Material.h>

namespace editor {

/*static const char* light_type_name(Light::Type type) {
	switch(type) {
		case Light::Directional:
			return "Directional";

		case Light::Point:
			return "Point";

		default:
		break;
	}
	return y_fatal("Unsupported light type.");
}*/

EntityView::EntityView(ContextPtr cptr) :
		Widget(ICON_FA_OBJECT_GROUP " Entities"),
		ContextLinked(cptr) {
}

void EntityView::add_light() {
	/*Scene::Ptr<Light> light = std::make_unique<Light>(Light::Point);
	context()->selection().set_selected(light.get());

	light->radius() = 100.0f;
	light->intensity() = 10000.0;
	context()->scene().scene().lights() << std::move(light);*/
}

void EntityView::paint_ui(CmdBufferRecorder&, const FrameToken&) {
	/*if(ImGui::Button(ICON_FA_PLUS, math::Vec2(24))) {
		ImGui::OpenPopup("Add entity");
	}

	if(ImGui::BeginPopup("Add entity")) {
		if(ImGui::MenuItem("Add light")) {
			add_light();
		}
		if(ImGui::MenuItem("Add renderable")) {
			auto instance = std::make_unique<StaticMeshInstance>(device()->device_resources()[DeviceResources::CubeMesh], device()->device_resources()[DeviceResources::EmptyMaterial]);
			instance->transform() = math::Transform<>(math::Vec3(0.0f, 0.0f, 0.0f), math::identity(), math::Vec3(0.1f));
			context()->scene().scene().static_meshes() << std::move(instance);
		}
		ImGui::EndPopup();
	}

	if(ImGui::TreeNode(ICON_FA_FOLDER " Renderables")) {
		for(const auto& r : context()->scene().scene().renderables()) {
			bool selected = r.get() == context()->selection().renderable();
			ImGui::Selectable(fmt(ICON_FA_QUESTION " %##%", type_name(*r), static_cast<void*>(r.get())).data(), &selected);
			if(selected) {
				context()->selection().set_selected(r.get());
			}
		}
		ImGui::TreePop();
	}

	if(ImGui::TreeNode(ICON_FA_FOLDER " Meshes")) {
		for(const auto& r : context()->scene().scene().static_meshes()) {
			bool selected = r.get() == context()->selection().renderable();
			ImGui::Selectable(fmt(ICON_FA_CUBE " %##%", type_name(*r), static_cast<void*>(r.get())).data(), &selected);
			if(selected) {
				context()->selection().set_selected(r.get());
			}
		}
		ImGui::TreePop();
	}

	if(ImGui::TreeNode(ICON_FA_FOLDER " Lights")) {
		for(const auto& l : context()->scene().scene().lights()) {
			bool selected = l.get() == context()->selection().light();
			ImGui::Selectable(fmt(ICON_FA_LIGHTBULB " %##%", light_type_name(l->type()), static_cast<void*>(l.get())).data(), &selected);
			if(selected) {
				context()->selection().set_selected(l.get());
			}
		}
		ImGui::TreePop();
	}*/

}

}
