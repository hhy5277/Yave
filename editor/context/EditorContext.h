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
#ifndef EDITOR_CONTEXT_EDITORCONTEXT_H
#define EDITOR_CONTEXT_EDITORCONTEXT_H

#include <yave/ecs/EntityWorld.h>

#include "EditorState.h"
#include "Settings.h"
#include "Selection.h"
#include "Ui.h"
#include "ThumbmailCache.h"
#include "PickingManager.h"
#include "EditorResources.h"
#include "Logs.h"

namespace editor {

class EditorContext : NonMovable, public DeviceLinked {

	public:
		EditorContext(DevicePtr dptr);
		~EditorContext();

		void flush_reload();

		void defer(core::Function<void()> func);
		void flush_deferred();

		void log_message(std::string_view msg, Log type);

		void save_world() const;
		void load_world();
		void new_world();


		void set_scene_view(SceneView* scene);
		void remove_scene_view(SceneView* scene);

		SceneView& scene_view();
		SceneView& default_scene_view();

		ecs::EntityWorld& world();


		const FileSystemModel* filesystem() const;
		const std::shared_ptr<FrameGraphResourcePool>& resource_pool() const;


		const EditorResources& resources() const;

		EditorState& editor_state();
		Settings& settings();
		Selection& selection();
		AssetLoader& loader();
		Ui& ui();
		ThumbmailCache& thumbmail_cache();
		PickingManager& picking_manager();
		AssetStore& asset_store();
		Logs& logs();

	private:
		std::unique_ptr<FileSystemModel> _filesystem;

		std::mutex _deferred_lock;
		core::Vector<core::Function<void()>> _deferred;
		bool _is_flushing_deferred = false;

		EditorResources _resources;

		std::shared_ptr<FrameGraphResourcePool> _resource_pool;

		std::shared_ptr<AssetStore> _asset_store;
		AssetLoader _loader;

		SceneView _default_scene_view;
		SceneView* _scene_view = nullptr;


		EditorState _editor_state;
		Settings _settings;
		Selection _selection;
		Ui _ui;
		ThumbmailCache _thumb_cache;
		PickingManager _picking_manager;
		Logs _logs;

		ecs::EntityWorld _world;
};

}

#endif // EDITOR_CONTEXT_EDITORCONTEXT_H
