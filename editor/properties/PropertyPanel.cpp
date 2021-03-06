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
#include <editor/properties/ComponentTraits.h>

#include <imgui/yave_imgui.h>

namespace editor {

PropertyPanel::PropertyPanel(ContextPtr cptr) :
		Widget(ICON_FA_WRENCH " Properties"),
		ContextLinked(cptr) {

	set_closable(false);
}

void PropertyPanel::paint(CmdBufferRecorder& recorder, const FrameToken& token) {
	Widget::paint(recorder, token);
}

void PropertyPanel::paint_ui(CmdBufferRecorder&, const FrameToken&) {
	if(!context()->selection().has_selected_entity()) {
		return;
	}

	ecs::EntityWorld& world = context()->world();
	ecs::EntityId id = context()->selection().selected_entity();

	for(const ecs::ComponentTypeIndex& type : world.component_types()) {
		if(world.has(id, type)) {
			component_widget(type, context(), id);
		}
	}
}

}
