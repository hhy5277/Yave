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

#include "EditorRenderer.h"


namespace editor {

EditorRenderer EditorRenderer::create(ContextPtr ctx, FrameGraph& framegraph, const SceneView& view, const math::Vec2ui& size, const std::shared_ptr<IBLData>& ibl_data, const Settings& settings) {
	EditorRenderer renderer;
	renderer.renderer = DefaultRenderer::create(framegraph, view, size, ibl_data);
	renderer.out = renderer.renderer.tone_mapping.tone_mapped;

	if(settings.enable_editor_entities) {
		renderer.entity_pass = EditorEntityPass::create(ctx, framegraph, view, renderer.renderer.gbuffer.depth, renderer.renderer.tone_mapping.tone_mapped);
		renderer.out = renderer.entity_pass.color;
	}

	return renderer;
}

}
