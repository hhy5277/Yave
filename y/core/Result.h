/*******************************
Copyright (c) 2016-2017 Grégoire Angerand

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
#ifndef Y_CORE_RESULT_H
#define Y_CORE_RESULT_H

#include <y/utils.h>

namespace y {
namespace core {

namespace detail {

template<typename T>
struct Ok : NonCopyable {

	Ok(T&& t) : _value(std::forward<T>(t)) {
	}

	Ok(Ok&& o) : _value(std::move(o._value)) {
	}

	const T& get() const {
		return _value;
	}

	T& get() {
		return _value;
	}

	private:
		T _value;
};

template<typename T>
struct Err : NonCopyable {

	Err(T&& t) : _err(std::forward<T>(t)) {
	}

	Err(Err&& e) : _err(std::move(e._err)) {
	}

	const T& get() const {
		return _err;
	}

	T& get() {
		return _err;
	}

	private:
		T _err;
};

template<>
struct Ok<void> : NonCopyable {
	Ok() {
	}

	Ok(Ok&&) {
	}

	void get() const {
	}
};


template<>
struct Err<void> : NonCopyable {
	Err() {
	}

	Err(Err&&) {
	}

	void get() const {
	}
};

}


inline auto Ok() {
	return detail::Ok<void>();
}

inline auto Err() {
	return detail::Err<void>();
}

template<typename T>
inline auto Ok(T&& t) {
	return detail::Ok<T>(std::forward<T>(t));
}

template<typename T>
inline auto Err(T&& e) {
	return detail::Err<T>(std::forward<T>(e));
}



template<typename T, typename E = void>
class Result : NonCopyable {

	static constexpr bool is_void = std::is_void<T>::value;
	using Err_t = detail::Err<E>;
	using Ok_t = detail::Ok<T>;

	enum State {
		eError,
		eOk
	};

	template<typename F, typename U>
	struct map_type {
		using type = decltype(make_one<F>()(make_one<U>()));
	};

	template<typename F>
	struct map_type<F, void> {
		using type = decltype(make_one<F>()());
	};


	public:
		template<typename U>
		Result(detail::Ok<U>&& v) : _state(eOk) {
			new(&_value) Ok_t(std::move(v));
		}

		template<typename U>
		Result(detail::Err<U>&& e) : _state(eError) {
			new(&_error) Err_t(std::move(e));
		}

		Result(Result&& other) : _state(other._state) {
			if(is_ok()) {
				new(&_value) Ok_t(std::move(other._value));
			} else {
				new(&_error) Err_t(std::move(other._error));
			}
		}


		~Result() {
			if(is_ok()) {
				_value.~Ok_t();
			} else {
				_error.~Err_t();
			}
		}


		bool is_error() const {
			return !is_ok();
		}

		bool is_ok() const {
			return _state == eOk;
		}

		auto&& unwrap() const {
			return expected("Unwrap failed.");
		}

		auto&& unwrap() {
			return expected("Unwrap failed.");
		}

		auto&& error() const {
			if(is_ok()) {
				fatal("Result is not an error.");
			}
			return _error.get();
		}

		auto&& error() {
			if(is_ok()) {
				fatal("Result is not an error.");
			}
			return _error.get();
		}

		auto&& expected(const char* err_msg) const {
			if(is_error()) {
				fatal(err_msg);
			}
			return _value.get();
		}

		auto&& expected(const char* err_msg) {
			if(is_error()) {
				fatal(err_msg);
			}
			return _value.get();
		}

		auto unwrap_or(const T& f) const {
			return is_ok() ? _value.get() : f;
		}

		template<typename F>
		Result<typename map_type<F, T>::type, E> map(const F& f) const {
			if(is_ok()) {
				return Ok(f(_value.get()));
			}
			return Err(_error.get());
		}


		template<typename F>
		Result<T, typename map_type<F, E>::type> map_err(const F& f) const {
			if(is_ok()) {
				return Ok(_value.get());
			}
			return Err(f(_error.get()));
		}

	protected:
		State _state;

		union {
			Ok_t _value;
			Err_t _error;
		};
};



}
}

#endif // Y_CORE_RESULT_H
