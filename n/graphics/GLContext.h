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

#ifndef N_GRAPHICS_CONTEXT_H
#define N_GRAPHICS_CONTEXT_H

#include <n/concurrent/SynchronizedQueue.h>
#include <n/core/Functor.h>
#include <n/math/Matrix.h>
#include <n/assets/Asset.h>

namespace n {
namespace graphics {

class ShaderCombinaison;
class FrameBuffer;

template<typename T>
class VertexArrayObject;

template<typename T>
class Material;

template<typename T>
struct MaterialData;

namespace internal {
	template<typename T>
	struct Material;

	struct ShaderProgram;
}

class GLContext
{
	public:
		enum HWInt
		{
			MaxFboAttachements = 0,
			MaxTextures = 1,
			MaxVertexAttribs = 2,
			Max = 3
		};

		static GLContext *getContext();


		math::Vec2ui getViewport() const;
		void setViewport(const math::Vec2ui &v);

		void addGLTask(const core::Functor<void()> &f);

		void setProjectionMatrix(const math::Matrix4<> &m);
		void setViewMatrix(const math::Matrix4<> &m);
		void setModelMatrix(const math::Matrix4<> &m);

		const math::Matrix4<> &getProjectionMatrix() const;
		const math::Matrix4<> &getViewMatrix() const;
		const math::Matrix4<> &getModelMatrix() const;

		bool processTasks();
		void finishTasks();

		void flush();

		void setDebugEnabled(bool deb);
		static bool checkGLError();

		const ShaderCombinaison *getShader() const {
			return shader;
		}

		const FrameBuffer *getFrameBuffer() const {
			return frameBuffer;
		}

		MaterialData<float> getMaterial() const;

		int getHWInt(HWInt hw) {
			return hwInts[hw];
		}

		const VertexArrayObject<float> &getScreen() const;


	private:
		friend class ShaderCombinaison;
		friend class ShaderProgram;
		friend class FrameBuffer;

		template<typename T>
		friend class Material;

		GLContext();
		~GLContext();

		concurrent::SynchronizedQueue<core::Functor<void()>> tasks;

		math::Matrix4<> projection;
		math::Matrix4<> view;
		math::Matrix4<> model;

		const ShaderCombinaison *shader;
		core::SmartPtr<internal::ShaderProgram> program;
		const FrameBuffer *frameBuffer;
		assets::Asset<internal::Material<float>> material;

		int hwInts[Max];

		math::Vec2ui viewport;

		mutable VertexArrayObject<float> *screen;
};

}
}


#endif // N_GRAPHICS_CONTEXT_H
