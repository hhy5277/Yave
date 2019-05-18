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
#ifndef Y_IO2_IO_H
#define Y_IO2_IO_H

#include <memory>
#include <y/core/Vector.h>
#include <y/core/Result.h>

namespace y {

namespace serde2 {
struct ReadableArchive;
}

namespace io2 {

using ReadUpToResult = core::Result<usize, usize>;
using ReadResult = core::Result<void, usize>;
using WriteResult = core::Result<void, usize>;
using FlushResult = core::Result<void>;

class Reader final {
	struct InnerBase {
		virtual ~InnerBase() = default;

		virtual bool at_end() const = 0;
		virtual ReadResult read(u8* data, usize bytes) = 0;
		virtual ReadUpToResult read_up_to(u8* data, usize max_bytes) = 0;
		virtual ReadUpToResult read_all(core::Vector<u8>& data) = 0;
	};

	template<typename T>
	struct Inner : InnerBase {
		Inner(T& t) : _inner(std::move(t)) {
		}

		Inner(T&& t) : _inner(std::move(t)) {
		}

		bool at_end() const override {
			return _inner.at_end();
		}

		ReadResult read(u8* data, usize bytes) override {
			return _inner.read(data, bytes);
		}

		ReadUpToResult read_up_to(u8* data, usize max_bytes) override {
			return _inner.read_up_to(data, max_bytes);
		}


		ReadUpToResult read_all(core::Vector<u8>& data) override {
			return _inner.read_all(data);
		}


		T _inner;
	};

	template<typename T>
	struct NotOwner : InnerBase {
		NotOwner(T& t) : _inner(t) {
		}

		bool at_end() const override {
			return _inner.at_end();
		}

		ReadResult read(u8* data, usize bytes) override {
			return _inner.read(data, bytes);
		}

		ReadUpToResult read_up_to(u8* data, usize max_bytes) override {
			return _inner.read_up_to(data, max_bytes);
		}

		ReadUpToResult read_all(core::Vector<u8>& data) override {
			return _inner.read_all(data);
		}

		T& _inner;
	};


	public:
		Reader() = default;

		template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Reader>>>
		Reader(T&& t) : _storage(std::make_unique<Inner<remove_cvref_t<T>>>(y_fwd(t))), _ref(_storage.get())  {
			static_assert(!std::is_base_of_v<serde2::ReadableArchive, std::decay_t<T>>);
		}

		template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Reader>>>
		Reader(T& t) : _storage(std::make_unique<NotOwner<remove_cvref_t<T>>>(t)), _ref(_storage.get()) {
			static_assert(!std::is_base_of_v<serde2::ReadableArchive, std::decay_t<T>>);
		}


		Reader(Reader&& other) {
			swap(other);
		}

		Reader& operator=(Reader&& other) {
			swap(other);
			return *this;
		}

		Reader(const Reader& other) : _ref(other._ref) {
		}

		Reader& operator=(const Reader& other) {
			_storage = nullptr;
			_ref = other._ref;
			return *this;
		}


		bool at_end() const {
			return _ref->at_end();
		}

		ReadResult read(u8* data, usize bytes) {
			return _ref->read(data, bytes);
		}

		ReadUpToResult read_up_to(u8* data, usize max_bytes) {
			return _ref->read_up_to(data, max_bytes);
		}

		ReadUpToResult read_all(core::Vector<u8>& data) {
			return _ref->read_all(data);
		}

		template<typename T>
		core::Result<void, usize> read_one(T& t) {
			static_assert(std::is_trivially_copyable_v<T>);
			return read(reinterpret_cast<u8*>(&t), sizeof(T));
		}

		template<typename T>
		core::Result<remove_cvref_t<T>, usize> read_one() {
			remove_cvref_t<T> t;
			if(auto r = read_one(t); !r) {
				return core::Err(r.error());
			}
			return core::Ok(std::move(t));
		}


		template<typename T>
		core::Result<void, usize> read_array(T* data, usize count) {
			static_assert(std::is_trivially_copyable_v<T>);
			return read(reinterpret_cast<u8*>(data), sizeof(T) * count);
		}

	private:
		void swap(Reader& other) {
			std::swap(_ref, other._ref);
			std::swap(_storage, other._storage);
		}

		std::unique_ptr<InnerBase> _storage;
		InnerBase* _ref = nullptr;
};

using ReaderRef = Reader;


class Writer final {
	struct InnerBase {
		virtual ~InnerBase() = default;

		virtual FlushResult flush() = 0;
		virtual WriteResult write(const u8* data, usize bytes) = 0;
	};

	template<typename T>
	struct Inner : InnerBase {
		Inner(T& t) : _inner(std::move(t)) {
		}

		Inner(T&& t) : _inner(std::move(t)) {
		}

		FlushResult flush() override {
			return _inner.flush();
		}

		WriteResult write(const u8* data, usize bytes) override {
			return _inner.write(data, bytes);
		}

		T _inner;
	};

	template<typename T>
	struct NotOwner : InnerBase {
		NotOwner(T& t) : _inner(t) {
		}

		FlushResult flush() override {
			return _inner.flush();
		}

		WriteResult write(const u8* data, usize bytes) override {
			return _inner.write(data, bytes);
		}

		T& _inner;
	};

	public:
		template<typename T>
		Writer(T&& t) : _inner(std::make_unique<Inner<T>>(y_fwd(t))) {
		}

		template<typename T>
		Writer(T& t) : _inner(std::make_unique<NotOwner<T>>(t))  {
		}

		FlushResult flush() {
			return _inner->flush();
		}

		WriteResult write(const u8* data, usize bytes) {
			return _inner->write(data, bytes);
		}

		template<typename T>
		WriteResult write_one(const T& t) {
			static_assert(std::is_trivially_copyable_v<T>);
			return write(reinterpret_cast<const u8*>(&t), sizeof(T));
		}

		template<typename T>
		WriteResult write_array(const T* data, usize count) {
			static_assert(std::is_trivially_copyable_v<T>);
			return write(reinterpret_cast<const u8*>(data), sizeof(T) * count);
		}

	private:
		std::unique_ptr<InnerBase> _inner;
};


}
}

#endif // Y_IO2_IO_H
