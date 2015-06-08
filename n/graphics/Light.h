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

#ifndef N_GRAPHICS_LIGHT
#define N_GRAPHICS_LIGHT

#include "Transformable.h"
#include "ShadowRenderer.h"
#include "Color.h"
#include <n/math/Volume.h>
#include <n/utils.h>

namespace n {
namespace graphics {

template<typename T = float>
class Light : public Movable<T>
{
	public:
		Light() : Movable<T>(), intensity(1), color(1) {
		}

		virtual ~Light() {
		}

		const Color<T> &getColor() const {
			return color;
		}

		void setColor(const Color<T> &c) {
			color = c.withLightness(1);
		}

		T getIntensity() const {
			return intensity;
		}

		void setIntensity(T t) {
			intensity = t;
		}

	private:
		T intensity;
		Color<T> color;
};

template<typename T = float>
class DirectionalLight : public Light<T>
{
	public:
		DirectionalLight() : Light<T>(), shadow(0) {
		}

		virtual ~DirectionalLight() override {
			delete shadow;
		}


	private:
		ShadowRenderer *shadow;
};

template<typename T = float>
class PointLight : public Light<T>
{
	using Transformable<T>::radius;
	public:
		PointLight() : Light<T>() {
			Transformable<T>::radius = 1;
		}

		virtual ~PointLight() {
		}

		void setRadius(T r) {
			radius = r;
		}

};

}
}


#endif // LIGHT

