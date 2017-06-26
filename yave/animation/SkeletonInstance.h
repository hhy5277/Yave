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
#ifndef YAVE_ANIMATION_SKELETONINSTANCE_H
#define YAVE_ANIMATION_SKELETONINSTANCE_H

#include <yave/mesh/Skeleton.h>
#include <yave/buffer/buffers.h>
#include <yave/bindings/DescriptorSet.h>

namespace yave {

class SkeletonInstance {

	public:
		SkeletonInstance() = default;

		SkeletonInstance(DevicePtr dptr, const Skeleton& skeleton);

		SkeletonInstance(SkeletonInstance&& other);
		SkeletonInstance& operator=(SkeletonInstance&& other);

		void update();

		const auto& descriptor_set() const {
			return _descriptor_set;
		}

	private:
		void swap(SkeletonInstance& other);

		const Skeleton* _skeleton = nullptr;

		TypedUniformBuffer<math::Transform<>, MemoryFlags::CpuVisible> _bone_transforms;
		DescriptorSet _descriptor_set;

};

}

#endif // YAVEE_ANIMATION_SKELETONINSTANCE_H
