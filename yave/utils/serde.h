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
#ifndef YAVE_UTILS_SERDE_H
#define YAVE_UTILS_SERDE_H

#include <yave/yave.h>
#include <yave/assets/AssetType.h>

#include <y/io2/io.h>
#include <y/serde2/serde.h>

namespace yave {

namespace fs {
static constexpr u32 magic_number = 0x65766179;
}

class AssetLoader;

using WritableAssetArchive = serde2::WritableArchive;

class ReadableAssetArchive : public serde2::ReadableArchiveBase<ReadableAssetArchive> {

	public:
		ReadableAssetArchive(io2::Reader& reader, AssetLoader& loader) :
				serde2::ReadableArchiveBase<ReadableAssetArchive>(reader),
				_loader(loader) {
		}

		ReadableAssetArchive(const io2::ReaderPtr& reader, AssetLoader& loader) : ReadableAssetArchive(*reader, loader) {
		}

		AssetLoader& loader() {
			return _loader;
		}

	private:
		AssetLoader& _loader;

};

#define yave_asset_serialize(...)									\
	y_serialize2_arc(yave::WritableAssetArchive, __VA_ARGS__)

#define yave_asset_deserialize(...)									\
	y_deserialize2_arc(yave::ReadableAssetArchive, __VA_ARGS__)


#define yave_asset_serde(...)										\
	yave_asset_serialize(__VA_ARGS__)								\
	yave_asset_deserialize(__VA_ARGS__)


}

#endif // YAVE_UTILS_SERDE_H
