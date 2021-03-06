/*******************************
Copyright (c) 2016-2019 Grégoire Angerand

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
#ifndef YAVE_GRAPHICS_IMAGES_IMAGE_H
#define YAVE_GRAPHICS_IMAGES_IMAGE_H

#include <yave/yave.h>
#include <yave/assets/AssetTraits.h>

#include "ImageBase.h"

namespace yave {

template<ImageUsage Usage, ImageType Type = ImageType::TwoD>
class Image : public ImageBase {

	static constexpr bool is_compatible(ImageUsage u) {
		return (uenum(Usage) & uenum(u)) == uenum(Usage);
	}

	static constexpr bool is_3d = Type == ImageType::ThreeD;

	template<typename T>
	math::Vec3ui to_3d_size(const T& size) {
		math::Vec3ui s(1);
		s.to<T::size()>() = size;
		return s;
	}

	public:
		using size_type = std::conditional_t<is_3d, math::Vec3ui, math::Vec2ui>;

		Image() = default;

		Image(DevicePtr dptr, ImageFormat format, const size_type& image_size) : ImageBase(dptr, format, Usage, to_3d_size(image_size)) {
			static_assert(is_attachment_usage(Usage) || is_storage_usage(Usage), "Texture images must be initilized.");
			static_assert(Type == ImageType::TwoD || is_storage_usage(Usage), "Only 2D images can be created empty.");
		}

		Image(DevicePtr dptr, const ImageData& data) : ImageBase(dptr, Usage, Type, data) {
			static_assert(is_texture_usage(Usage), "Only texture images can be initilized.");
		}

		template<ImageUsage U, typename = std::enable_if_t<is_compatible(U)>>
		Image(Image<U, Type>&& other) {
			static_assert(is_compatible(U));
			ImageBase::operator=(std::move(other));
		}

		template<ImageUsage U, typename = std::enable_if_t<is_compatible(U)>>
		Image& operator=(Image<U, Type>&& other) {
			static_assert(is_compatible(U));
			ImageBase::operator=(std::move(other));
			return *this;
		}

		const size_type& size() const {
			return image_size().to<size_type::size()>();
		}
};

using Texture = Image<ImageUsage::TextureBit>;
using StorageTexture = Image<ImageUsage::TextureBit | ImageUsage::StorageBit>;
using DepthAttachment = Image<ImageUsage::DepthBit>;
using ColorAttachment = Image<ImageUsage::ColorBit>;
using DepthTextureAttachment = Image<ImageUsage::DepthBit | ImageUsage::TextureBit>;
using ColorTextureAttachment = Image<ImageUsage::ColorBit | ImageUsage::TextureBit>;

using Cubemap = Image<ImageUsage::TextureBit, ImageType::Cube>;

YAVE_DECLARE_ASSET_TRAITS(Texture, ImageData, AssetType::Image);

}

#endif // YAVE_GRAPHICS_IMAGES_IMAGE_H
