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

#include "SimpleMaterialData.h"

#include <y/io/File.h>

namespace yave {


SimpleMaterialData::SimpleMaterialData(std::array<AssetPtr<Texture>, texture_count>&& textures) :
		_textures(std::move(textures)) {
}

core::Result<SimpleMaterialData> SimpleMaterialData::load(io::ReaderRef reader, AssetLoader& loader) noexcept {
	try {
		SimpleMaterialHeader().deserialize(reader);
		SimpleMaterialData data;
		for(auto& tex : data._textures) {
			auto id = serde::deserialized<AssetId>(reader);
			if(id != AssetId::invalid_id()) {
				tex = std::move(loader.load<Texture>(id).or_throw("Unable to load texture."));
			}
		}
		return core::Ok(data);
	} catch(...) {
	}
	return core::Err();
}

core::Result<SimpleMaterialData> SimpleMaterialData::load(ReadableAssetArchive& arc) noexcept {
	SimpleMaterialHeader header;
	if(!arc(header)) {
		return core::Err();
	}
	SimpleMaterialData data;
	for(auto& tex : data._textures) {
		AssetId id;
		if(arc(id) && id != AssetId::invalid_id()) {
			if(auto t = arc.loader().load<Texture>(id)) {
				tex = std::move(t.unwrap());
			} else {
				return core::Err();
			}
		}
	}
	return core::Ok(data);
}



SimpleMaterialData& SimpleMaterialData::set_texture(Textures type, AssetPtr<Texture> tex) {
	_textures[usize(type)] = std::move(tex);
	return *this;
}

bool SimpleMaterialData::is_empty() const {
	return std::all_of(_textures.begin(), _textures.end(), [](const auto& tex) { return !tex; });
}

const AssetPtr<Texture>& SimpleMaterialData::operator[](Textures tex) const {
	return _textures[usize(tex)];
}

const std::array<AssetPtr<Texture>, SimpleMaterialData::texture_count>& SimpleMaterialData::textures() const {
	return  _textures;
}

std::array<AssetId, SimpleMaterialData::texture_count> SimpleMaterialData::texture_ids() const {
	std::array<AssetId, texture_count> ids;
	std::transform(_textures.begin(), _textures.end(), ids.begin(), [](const auto& tex) { return tex.id(); });
	return ids;
}


}
