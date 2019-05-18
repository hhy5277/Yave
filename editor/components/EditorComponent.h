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
#ifndef EDITOR_COMPONENTS_EDITORCOMPONENT_H
#define EDITOR_COMPONENTS_EDITORCOMPONENT_H

#include <editor/editor.h>
#include <yave/ecs/ecs.h>
#include <y/core/String.h>

#include <yave/utils/serde.h>

namespace editor {

class EditorComponent {
	public:
		EditorComponent() = default;
		EditorComponent(std::string_view name);

		const core::String& name() const;
		bool is_visible() const;

		y_serde2(_name, _visible)

	private:
		core::String _name;
		bool _visible = true;
};

}

#endif // EDITOR_COMPONENTS_EDITORCOMPONENT_H
