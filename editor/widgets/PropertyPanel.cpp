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

#include "PropertyPanel.h"

#include <editor/context/EditorContext.h>
#include <editor/widgets/AssetSelector.h>
#include <editor/components/EditorComponent.h>

#include <yave/components/RenderableComponent.h>
#include <yave/components/LightComponent.h>

#include <imgui/imgui_yave.h>

namespace editor {

PropertyPanel::PropertyPanel(ContextPtr cptr) :
		Widget(ICON_FA_WRENCH " Properties"),
		ContextLinked(cptr) {
	set_closable(false);
}

void PropertyPanel::paint(CmdBufferRecorder& recorder, const FrameToken& token) {
	Widget::paint(recorder, token);

	/*if(Transformable* selected = context()->selection().selected()) {
		auto end_pos = context()->scene().to_window_pos(selected->position());
		auto start_pos = position() + math::Vec2(0.0f, 12.0f);

		u32 color = ImGui::GetColorU32(ImGuiCol_Text) | 0xFF000000;

		// ImGui::GetWindowDrawList()->AddLine(start_pos, end_pos, color);

		auto point = math::Vec2(std::copysign(128.0f, end_pos.x() - start_pos.x()), 0.0f);
		ImGui::GetWindowDrawList()->AddBezierCurve(start_pos, start_pos + point, end_pos - point, end_pos, color, 2.0f);
	}*/
}

void PropertyPanel::paint_ui(CmdBufferRecorder&, const FrameToken&) {
	if(!context()->selection().has_selected_entity()) {
		return;
	}

	ecs::EntityWorld& world = context()->world();
	ecs::EntityId id = context()->selection().selected_entity();

	if(world.has<EditorComponent>(id)) {
		if(ImGui::CollapsingHeader("Entity", ImGuiTreeNodeFlags_DefaultOpen)) {
			panel(world.component<EditorComponent>(id));
		}
	}

	if(world.has<RenderableComponent>(id)) {
		if(ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto& component = world.component<RenderableComponent>(id);
			transformable_panel(*component.get());
			panel(component);
		}
	}

	if(world.has<LightComponent>(id)) {
		if(ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto& component = world.component<LightComponent>(id);
			transformable_panel(component.light());
			panel(component);
		}
	}

	//ImGui::Text("Position: %f, %f, %f", sel->position().x(), sel->position().y(), sel->position().z());*/
}


void PropertyPanel::panel(EditorComponent& component) {
	const core::String& name = component.name();

	std::array<char, 1024> name_buffer;
	std::fill(name_buffer.begin(), name_buffer.end(), 0);
	std::copy(name.begin(), name.end(), name_buffer.begin());

	if(ImGui::InputText("Name", name_buffer.begin(), name_buffer.size())) {
		component.set_name(name_buffer.begin());
	}

}

void PropertyPanel::panel(RenderableComponent& component) {
	if(StaticMeshInstance* instance = dynamic_cast<StaticMeshInstance*>(component.get())) {
		Y_TODO(use ECS to ensure that we dont modify a deleted object)
		if(!ImGui::CollapsingHeader("Static mesh")) {
			return;
		}

		auto clean_name = [=](auto&& n) { return context()->asset_store().filesystem()->filename(n); };

		// material
		{
			core::String name = context()->asset_store().name(instance->material().id()).map(clean_name).unwrap_or("No material");
			if(ImGui::Button(ICON_FA_FOLDER_OPEN)) {
				context()->ui().add<AssetSelector>(AssetType::Material)->set_selected_callback(
					[=, ctx = context()](AssetId id) {
						if(auto material = ctx->loader().load<Material>(id)) {
							instance->material() = material.unwrap();
						}
						return true;
					});
			}
			ImGui::SameLine();
			ImGui::InputText("Material", name.data(), name.size(), ImGuiInputTextFlags_ReadOnly);
		}

		// mesh
		{
			core::String name = context()->asset_store().name(instance->mesh().id()).map(clean_name).unwrap_or("No mesh");
			if(ImGui::Button(ICON_FA_FOLDER_OPEN)) {
				context()->ui().add<AssetSelector>(AssetType::Mesh)->set_selected_callback(
					[=, ctx = context()](AssetId id) {
						if(auto mesh = ctx->loader().load<StaticMesh>(id)) {
							instance->mesh() = mesh.unwrap();
						}
						return true;
					});
			}
			ImGui::SameLine();
			ImGui::InputText("Mesh", name.data(), name.size(), ImGuiInputTextFlags_ReadOnly);
		}
	}
}


void PropertyPanel::panel(LightComponent& component) {
	Light& light = component.light();

	int color_flags = ImGuiColorEditFlags_NoSidePreview |
					  // ImGuiColorEditFlags_NoSmallPreview |
					  ImGuiColorEditFlags_NoAlpha |
					  ImGuiColorEditFlags_Float |
					  ImGuiColorEditFlags_InputRGB;

	math::Vec4 color(light.color(), 1.0f);
	if(ImGui::ColorButton("Color", color, color_flags)) {
		ImGui::OpenPopup("Color");
	}
	ImGui::SameLine();
	ImGui::Text("Light color");

	if(ImGui::BeginPopup("Color")) {
		ImGui::ColorPicker3("##lightpicker", light.color().begin(), color_flags);

		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::Text("temperature slider should be here");
		ImGui::EndGroup();

		ImGui::EndPopup();
	}

	ImGui::DragFloat("Intensity", &light.intensity());
	ImGui::DragFloat("Radius", &light.radius());
}

void PropertyPanel::transformable_panel(Transformable& transformable) {
	if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto [pos, rot, scale] = transformable.transform().decompose();

		// position
		{
			ImGui::BeginGroup();
			ImGui::InputFloat3("Position", pos.data(), "%.2f");
			ImGui::EndGroup();
		}

		// rotation
		{
			auto dedouble_quat = [](math::Quaternion<> q) {
					return q.x() < 0.0f ? -q.as_vec() : q.as_vec();
				};
			auto is_same_angle = [&](math::Vec3 a, math::Vec3 b) {
					auto qa = math::Quaternion<>::from_euler(a * math::to_rad(1.0f));
					auto qb = math::Quaternion<>::from_euler(b * math::to_rad(1.0f));
					return (dedouble_quat(qa) - dedouble_quat(qb)).length2() < 0.0001f;
				};

			math::Vec3 euler = rot.to_euler() * math::to_deg(1.0f);
			if(is_same_angle(euler, _euler)) {
				euler = _euler;
			}

			float speed = 1.0f;
			ImGui::BeginGroup();
			ImGui::DragFloat("Yaw", &euler[math::Quaternion<>::YawIndex], speed, -180.0f, 180.0f, "%.2f");
			ImGui::DragFloat("Pitch", &euler[math::Quaternion<>::PitchIndex], speed, -180.0f, 180.0f, "%.2f");
			ImGui::DragFloat("Roll", &euler[math::Quaternion<>::RollIndex], speed, -180.0f, 180.0f, "%.2f");
			ImGui::EndGroup();

			if(!is_same_angle(euler, _euler)) {
				_euler = euler;
				rot = math::Quaternion<>::from_euler(euler * math::to_rad(1.0f));
			}
		}

		transformable.transform() = math::Transform<>(pos, rot, scale);
	}
}

}
