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

#include "MeshImporter.h"

#include <editor/context/EditorContext.h>
#include <editor/import/transforms.h>
#include <editor/utils/assets.h>

#include <editor/components/EditorComponent.h>

#include <yave/components/TransformableComponent.h>
#include <yave/components/StaticMeshComponent.h>
#include <yave/entities/entities.h>

#include <y/io2/Buffer.h>

#include <imgui/yave_imgui.h>

namespace editor {

MeshImporter::MeshImporter(ContextPtr ctx, const core::String& import_path) :
		Widget("Mesh importer"),
		ContextLinked(ctx),
		_import_path(import_path) {

	_browser.set_has_parent(true);
	_browser.set_extension_filter(import::supported_scene_extensions());
	_browser.set_canceled_callback([this] { close(); return true; });
	_browser.set_selected_callback([this](const auto& filename) {
			//import_async(filename);
			_filename = filename;
			_state = State::Settings;
			return true;
		});
}

void MeshImporter::paint_ui(CmdBufferRecorder& recorder, const FrameToken& token)  {
	using import::SceneImportFlags;

	if(_state == State::Browsing) {
		_browser.paint(recorder, token);
	} else if(_state == State::Settings) {
		bool import_meshes = (_flags & SceneImportFlags::ImportMeshes) == SceneImportFlags::ImportMeshes;
		bool import_anims = (_flags & SceneImportFlags::ImportAnims) == SceneImportFlags::ImportAnims;
		bool import_images = (_flags & SceneImportFlags::ImportImages) == SceneImportFlags::ImportImages;
		bool import_materials = (_flags & SceneImportFlags::ImportMaterials) == SceneImportFlags::ImportMaterials;
		bool import_objects = (_flags & SceneImportFlags::ImportObjects) == SceneImportFlags::ImportObjects;

		ImGui::Checkbox("Import meshes", &import_meshes);
		ImGui::Checkbox("Import animations", &import_anims);
		ImGui::Checkbox("Import images", &import_images);
		ImGui::Checkbox("Import materials", &import_materials);
		ImGui::Separator();
		ImGui::Checkbox("Import objects and create world", &import_objects);

		_flags = (import_meshes ? SceneImportFlags::ImportMeshes : SceneImportFlags::None) |
				 (import_anims ? SceneImportFlags::ImportAnims : SceneImportFlags::None) |
				 (import_images ? SceneImportFlags::ImportImages : SceneImportFlags::None) |
				 (import_materials ? SceneImportFlags::ImportMaterials : SceneImportFlags::None) |
				 (import_objects ? SceneImportFlags::ImportObjects : SceneImportFlags::None)
			;

		ImGui::Separator();

		const char* axes[] = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
		if(ImGui::BeginCombo("Forward", axes[_forward_axis])) {
			for(usize i = 0; i != 6; ++i) {
				if(ImGui::Selectable(axes[i])) {
					_forward_axis = i;
					if(_forward_axis / 2 == _up_axis / 2) {
						_up_axis = (_up_axis + 4) % 6;
					}
				}
			}
			ImGui::EndCombo();
		}
		if(ImGui::BeginCombo("Up", axes[_up_axis])) {
			for(usize i = 0; i != 6; ++i) {
				if(ImGui::Selectable(axes[i])) {
					_up_axis = i;
					if(_forward_axis / 2 == _up_axis / 2) {
						_forward_axis = (_forward_axis + 4) % 6;
					}
				}
			}
			ImGui::EndCombo();
		}


		if(ImGui::Button("Ok")) {
			_state = State::Importing;
			_import_future = std::async(std::launch::async, [=] {
				return import::import_scene(_filename, _flags);
			});
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel")) {
			close();
		}
	} else {
		if(done_loading()) {
			try {
				import(std::move(_import_future.get()));
			} catch(std::exception& e) {
				log_msg(fmt("Unable to import scene: %" , e.what()), Log::Error);
				_browser.show();
			}
			close();
		} else {
			ImGui::Text("Loading...");
		}
	}
}

bool MeshImporter::done_loading() const {
	return _import_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void MeshImporter::import(import::SceneData scene) {
	y_profile();

	auto make_full_name = [this](std::string_view import_path, std::string_view name) {
			core::String path = _import_path;
			if(!import_path.empty()) {
				path = context()->asset_store().filesystem()->join(path, import_path);
			}
			return context()->asset_store().filesystem()->join(path, name);
		};

	auto import_asset = [&](const auto& asset, std::string_view name) {
			log_msg(fmt("Saving asset as \"%\"", name));
			y_profile_zone("asset import");
			io2::Buffer buffer;
			WritableAssetArchive ar(buffer);
			if(asset.serialize(ar)) {
				if(context()->asset_store().import(buffer, name)) {
					return;
				}
				log_msg(fmt("Unable import \"%\"", name), Log::Error);
			} else {
				log_msg(fmt("Unable serialize \"%\"", name), Log::Error);
			}
		};

	auto import_assets = [&](const auto& assets, std::string_view import_path) {
			for(const auto& a : assets) {
				core::String name = make_full_name(import_path, a.name());
				import_asset(a.obj(), name);
			}
		};

	auto compile_material = [&](const import::MaterialData& data, std::string_view texture_include_path) {
			SimpleMaterialData material;
			for(usize i = 0; i != SimpleMaterialData::texture_count; ++i) {
				if(!data.textures[i].is_empty()) {
					core::String tex_full_name = make_full_name(texture_include_path, data.textures[i]);
					if(auto texture = context()->loader().load<Texture>(tex_full_name)) {
						material.set_texture(SimpleMaterialData::Textures(i), std::move(texture.unwrap()));
					} else {
						log_msg(fmt("Unable to load texture \"%\"", tex_full_name), Log::Error);
					}
				}
			}
			return material;
		};


	if(_forward_axis != 0 || _up_axis != 4) {
		math::Vec3 axes[] = {{1.0f, 0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f},
							 {0.0f, 1.0f, 0.0f}, { 0.0f, -1.0f,  0.0f},
							 {0.0f, 0.0f, 1.0f}, { .0f,  0.0f, -1.0f}};
		math::Transform<> transform;

		math::Vec3 forward = axes[_forward_axis];
		math::Vec3 up = axes[_up_axis];
		Y_TODO(try to auto detect handedness)
		transform.set_basis(forward, -forward.cross(up), up);

		for(auto& mesh : scene.meshes) {
			mesh = import::transform(mesh.obj(), transform.transposed());
		}
	}



	{
		bool separate_folders = !scene.meshes.is_empty() + !scene.animations.is_empty() + !scene.images.is_empty();
		core::String mesh_import_path = separate_folders ? "meshes" : "";
		core::String animations_import_path = separate_folders ? "animations" : "";
		core::String image_import_path = separate_folders ? "images" : "";
		core::String material_import_path = separate_folders ? "materials" : "";
		core::String world_import_path = "";

		{
			import_assets(scene.meshes, mesh_import_path);
			import_assets(scene.animations, animations_import_path);
			import_assets(scene.images, image_import_path);
		}

		{
			auto materials = core::vector_with_capacity<Named<SimpleMaterialData>>(scene.materials.size());
			std::transform(scene.materials.begin(), scene.materials.end(), std::back_inserter(materials),
						   [&](const auto& m) { return Named(m.name(), compile_material(m.obj(), image_import_path)); });

			import_assets(materials, material_import_path);
		}

		{
			using import::SceneImportFlags;
			if((_flags & SceneImportFlags::ImportObjects) == SceneImportFlags::ImportObjects) {
				y_profile_zone("import world");

				ecs::EntityWorld world;
				for(const auto& named_obj : scene.objects) {
					const auto& object = named_obj.obj();

					auto material = context()->loader().load<Material>(make_full_name(material_import_path, object.material));
					auto mesh = context()->loader().load<StaticMesh>(make_full_name(mesh_import_path, object.mesh));

					if(!material) {
						log_msg(fmt("Unable to load material \"%\"", object.material), Log::Error);
						continue;
					}

					if(!mesh) {
						log_msg(fmt("Unable to load mesh \"%\"", object.mesh), Log::Error);
						continue;
					}

					ecs::EntityId entity = world.create_entity(StaticMeshArchetype());
					world.create_component<EditorComponent>(entity, named_obj.name());
					StaticMeshComponent* static_mesh = world.component<StaticMeshComponent>(entity);
					static_mesh->material() = material.unwrap();
					static_mesh->mesh() = mesh.unwrap();
				}

				core::String name = make_full_name(world_import_path, "world");
				import_asset(world, name);
			}
		}
	}

	context()->ui().refresh_all();
}

}
