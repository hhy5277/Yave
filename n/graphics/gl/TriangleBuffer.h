/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_GRAPHICS_GL_TRIANGLEBUFFER
#define N_GRAPHICS_GL_TRIANGLEBUFFER

#include "VertexBuffer.h"
#include <n/defines.h>
#ifndef N_NO_GL

namespace n {
namespace graphics {
namespace gl {

template<typename T>
class VertexArrayObject;

template<typename T = float>
class TriangleBuffer
{
	struct TriData
	{
		typedef const uint * const_iterator;
		TriData(uint a, uint b, uint c) : vts{a, b, c} {
		}

		uint vts[3];

		const_iterator begin() const {
			return vts;
		}

		const_iterator end() const {
			return vts + 4;
		}

	};

	public:
		class FreezedTriangleBuffer
		{
			public:
				core::Array<uint> indexes;
				core::Array<Vertex<T>> vertices;

			private:
				friend class TriangleBuffer;
				FreezedTriangleBuffer(const TriangleBuffer &b) : indexes(b.trianglesData), vertices(b.vertices.getVertices()) {
					for(const TriData &t : b.trianglesData) {
						math::Vec<3, T> edges[] = {vertices[t.vts[1]].getPosition() - vertices[t.vts[0]].getPosition(), vertices[t.vts[2]].getPosition() - vertices[t.vts[0]].getPosition()};
						T dt[] = {vertices[t.vts[1]].getTexCoord().y() - vertices[t.vts[0]].getTexCoord().y(), vertices[t.vts[2]].getTexCoord().y() - vertices[t.vts[0]].getTexCoord().y()};
						T db[] = {vertices[t.vts[1]].getTexCoord().x() - vertices[t.vts[0]].getTexCoord().x(), vertices[t.vts[2]].getTexCoord().x() - vertices[t.vts[0]].getTexCoord().x()};
						T scale = T(1) / ((db[0] * dt[1]) - (db[1] * dt[0]));
						math::Vec<3, T> ta = ((edges[0] * dt[1]) - (edges[1] * dt[0])) * scale;
						vertices[t.vts[0]].getTangent() += ta;
						vertices[t.vts[1]].getTangent() += ta;
						vertices[t.vts[2]].getTangent() += ta;
					}
					for(Vertex<T> &v : vertices) {
						v.getTangent().normalize();
					}
				}


		};

		class Triangle
		{
			public:
				const Vertex<T> &operator[](uint i) const {
					return getVertex(i);
				}

				const Vertex<T> &getVertex(uint i) const {
					return buffer->vertices[buffer->trianglesData[index][i]];
				}

			private:
				friend class TriangleBuffer;
				Triangle(uint i, TriangleBuffer<T> *b) : index(i), buffer(b) {
				}

				Vertex<T> &getVertex(uint i) {
					buffer->modified = true;
					return buffer->vertices[buffer->trianglesData[index][i]];
				}

				uint index;
				TriangleBuffer<T> *buffer;
		};

		TriangleBuffer() {
		}

		Triangle append(const Vertex<T> &a, const Vertex<T> &b, const Vertex<T> &c) {
			trianglesData.append(TriData(vertices.append(a), vertices.append(b), vertices.append(c)));
			Triangle tr(trianglesData.size() - 1, this);
			triangles.append(tr);
			return tr;
		}

		typename core::Array<Triangle>::iterator begin() {
			return triangles.begin();
		}

		typename core::Array<Triangle>::iterator end() {
			return triangles.end();
		}

		typename core::Array<Triangle>::const_iterator begin() const {
			return triangles.begin();
		}

		typename core::Array<Triangle>::const_iterator end() const {
			return triangles.end();
		}

		const core::Array<Triangle> &getTriangles() const {
			return triangles;
		}

		const VertexBuffer<T> &getVertices() const {
			return vertices;
		}

		const FreezedTriangleBuffer freezed() const {
			return FreezedTriangleBuffer(*this);
		}

	private:
		template<typename U>
		friend class VertexArrayObject;

		core::Array<TriData> trianglesData;
		core::Array<Triangle> triangles;

		VertexBuffer<T> vertices;
};

}
}
}

#endif

#endif // N_GRAPHICS_GL_TRIANGLEBUFFER

