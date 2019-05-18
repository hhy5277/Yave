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
#ifndef Y_IO2_FILE_H
#define Y_IO2_FILE_H

#include "io.h"

#include <y/core/String.h>

#include <cstdio>

namespace y {
namespace io2 {

class File : NonCopyable {

	public:
		File() = default;
		~File();

		File(File&& other);
		File& operator=(File&& other);

		static core::Result<File> create(const core::String& name);
		static core::Result<File> open(const core::String& name);

		static  core::Result<void> copy(ReaderRef src, const core::String& dst);

		usize size() const;
		usize remaining() const;

		bool is_open() const;
		bool at_end() const;

		void seek(usize byte);

		ReadResult read(u8* data, usize bytes);
		ReadUpToResult read_up_to(u8* data, usize max_bytes);
		ReadUpToResult read_all(core::Vector<u8>& data);

		WriteResult write(const u8* data, usize bytes);
		FlushResult flush();

	private:
		File(std::FILE* f);

		void swap(File& other);

		std::FILE* _file = nullptr;
};

}
}

#endif // Y_IO2_FILE_H
