/*******************************
Copyright (c) 2016-2017 Gr�goire Angerand

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
#ifndef YAVE_BUFFERS_BUFFERS_H
#define YAVE_BUFFERS_BUFFERS_H

#include "TypedSubBuffer.h"
#include <yave/meshes/Vertex.h>

namespace yave {

template<MemoryType Memory = MemoryType::DeviceLocal>
using AttribBuffer = Buffer<BufferUsage::AttributeBit, Memory>;

template<typename T, MemoryType Memory = MemoryType::DeviceLocal>
using TypedAttribBuffer = TypedBuffer<T, BufferUsage::AttributeBit, Memory>;


template<MemoryType Memory = MemoryType::DeviceLocal>
using UniformBuffer = Buffer<BufferUsage::UniformBit, Memory>;

template<typename T, MemoryType Memory = MemoryType::DeviceLocal>
using TypedUniformBuffer = TypedBuffer<T, BufferUsage::UniformBit, Memory>;



template<MemoryType Memory = MemoryType::DeviceLocal>
using TriangleBuffer = TypedBuffer<IndexedTriangle, BufferUsage::IndexBit,Memory>;

template<MemoryType Memory = MemoryType::DeviceLocal>
using VertexBuffer = TypedBuffer<Vertex, BufferUsage::AttributeBit, Memory>;

template<MemoryType Memory = MemoryType::DeviceLocal>
using SkinnedVertexBuffer = TypedBuffer<SkinnedVertex, BufferUsage::AttributeBit, Memory>;

template<MemoryType Memory = MemoryType::DeviceLocal>
using IndirectBuffer = TypedBuffer<vk::DrawIndexedIndirectCommand, BufferUsage::IndirectBit, Memory>;



using AttribSubBuffer = SubBuffer<BufferUsage::AttributeBit>;
using TriangleSubBuffer = TypedSubBuffer<IndexedTriangle, BufferUsage::IndexBit>;
using VertexSubBuffer = TypedSubBuffer<Vertex, BufferUsage::AttributeBit>;
using IndirectSubBuffer = TypedSubBuffer<vk::DrawIndexedIndirectCommand, BufferUsage::IndirectBit>;

}

#endif // YAVE_BUFFERS_BUFFERS_H