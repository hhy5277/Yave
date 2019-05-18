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
#ifndef YAVE_MATERIAL_MATERIAL_H
#define YAVE_MATERIAL_MATERIAL_H

#include "MaterialTemplate.h"
#include "SimpleMaterialData.h"

namespace yave {

class Material final : NonCopyable {

	public:
		Material() = default;
		Material(DevicePtr dptr, SimpleMaterialData&& data);
		Material(const MaterialTemplate* tmp, SimpleMaterialData&& data = SimpleMaterialData());

		const MaterialTemplate* mat_template() const;

		const SimpleMaterialData& data() const;
		const DescriptorSetBase& descriptor_set() const;

		DevicePtr device() const;

	private:
		NotOwner<const MaterialTemplate*> _template = nullptr;

		DescriptorSet _set;

		SimpleMaterialData _data;
};

template<>
struct AssetTraits<Material> {
	static constexpr bool is_asset = true;

	static constexpr AssetType type = AssetType::Material;

	using Result = core::Result<Material>;

	static Result load_asset(io::ReaderRef reader, AssetLoader& loader) noexcept {
		return SimpleMaterialData::load(reader, loader).map([&](auto&& data) { return Material(loader.device(), std::move(data)); });
	}
	static Result load_asset(AssetReadableArchive& arc) noexcept {
		return SimpleMaterialData::load(arc).map([&](auto&& data) { return Material(arc.loader().device(), std::move(data)); });
	}
	static Result load_asset(io2::ReaderRef reader, AssetLoader& loader) noexcept {
		AssetReadableArchive ar(reader, loader);
		return load_asset(ar);
	}
};

}


#endif // YAVE_MATERIAL_MATERIAL_H
