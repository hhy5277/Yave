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
#ifndef Y_SERDE2_SERDE_H
#define Y_SERDE2_SERDE_H

#include <y/core/Result.h>
#include <y/core/Functor.h>

#define Y_FIRST(a, ...) a
#define Y_SECOND(a, b, ...) b

#define Y_EMPTY()

#define Y_DEFER1(m) m Y_EMPTY()
#define Y_DEFER2(m) m Y_EMPTY Y_EMPTY()()
#define Y_DEFER3(m) m Y_EMPTY Y_EMPTY Y_EMPTY()()()
#define Y_DEFER4(m) m Y_EMPTY Y_EMPTY Y_EMPTY Y_EMPTY()()()()

#define Y_IS_PROBE(...) Y_SECOND(__VA_ARGS__, 0)
#define Y_PROBE() ~, 1

#define Y_CAT(a,b) a ## b

#define Y_NOT(x) Y_IS_PROBE(Y_CAT(Y_NOT_, x))
#define Y_NOT_0 Y_PROBE()

#define Y_BOOL(x) Y_NOT(Y_NOT(x))

#define IF_ELSE(condition) _IF_ELSE(Y_BOOL(condition))
#define _IF_ELSE(condition) Y_CAT(_IF_, condition)

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)             _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__

#define Y_HAS_ARGS(...) Y_BOOL(Y_FIRST(Y_END_OF_ARGUMENTS __VA_ARGS__)())
#define Y_END_OF_ARGUMENTS() 0


#define Y_MACRO_MAP(m, first, ...)									\
	m(first)														\
	IF_ELSE(Y_HAS_ARGS(__VA_ARGS__))								\
		(Y_DEFER2(Y_MACRO_MAP_)()(m, __VA_ARGS__))					\
		(/* Do nothing, just terminate */)
	
	
#define Y_MACRO_MAP_() Y_MACRO_MAP

namespace y {
namespace serde2 {

using Result = core::Result<void>;

#define y_serde2_unfold_arg(N)										\
	if(!_y_serde_arc(N)) { return y::core::Err(); }


//__VA_OPT__(DEFER(y_serde2_unfold_args)()(__VA_ARGS__))

#define y_serialize2(...)											\
	template<typename Arc>											\
	y::serde2::Result serialize(Arc&& _y_serde_arc) const {			\
		return _y_serde_arc(__VA_ARGS__);							\
	}

#define y_deserialize2(...)											\
	template<typename Arc>											\
	y::serde2::Result deserialize(Arc&& _y_serde_arc) {				\
		Y_REC_MACRO(Y_MACRO_MAP(y_serde2_unfold_arg, __VA_ARGS__))	\
		return y::core::Ok();										\
	}

#define y_serde2(...)												\
	y_serialize2(__VA_ARGS__)										\
	y_deserialize2(__VA_ARGS__)


#define y_serde_deser_compat()									\
	void deserialize(y::io::ReaderRef r) {						\
		y::io2::Reader reader(r);								\
		y::serde2::ReadableArchive ar(reader);					\
		ar(*this).or_throw("serde2");							\
	}															\
	static auto _y_serde2_self_type_helper() ->					\
		std::remove_reference<decltype(*this)>::type;			\
	static auto deserialized(y::io::ReaderRef r) {				\
		y::io2::Reader reader(r);								\
		y::serde2::ReadableArchive ar(reader);					\
		decltype(_y_serde2_self_type_helper()) t;				\
		ar(t).or_throw("serde2");								\
		return t;												\
	}

#define y_serde_ser_compat()									\
	void serialize(y::io::WriterRef w) const {					\
		y::io2::Writer writer(w);								\
		y::serde2::WritableArchive ar(writer);					\
		serialize(ar).or_throw("serde2");						\
	}

#define y_serde_compat()										\
	y_serde_deser_compat()										\
	y_serde_ser_compat()



namespace detail {
template<typename... Args>
class Checker {
	public:
		Checker(Args&&... args) : _t(std::make_tuple(y_fwd(args)...)) {
		}

		template<typename Arc>
		Result deserialize(Arc&& ar) {
			decltype(_t) t;
			if(ar(t) && t == _t) {
				return core::Ok();
			}
			return core::Err();
		}

		template<typename Arc>
		Result serialize(Arc&& ar) const {
			return ar(_t);
		}

	private:
		std::tuple<remove_cvref_t<Args>...> _t;
};

template<typename T>
class Function {
	public:
		Function(T&& t) : _t(y_fwd(t)) {
		}

		template<typename Arc>
		Result deserialize(Arc&& ar) {
			using ret_t = typename function_traits<T>::return_type;
			using args_t = typename function_traits<T>::argument_pack ;
			if constexpr(std::is_convertible_v<ret_t, Result> && std::is_convertible_v<decltype(ar), args_t>) {
				return _t(ar);
			} else {
				args_t args;
				if(!ar(args)) {
					return core::Err();
				}
				std::apply(_t, std::move(args));
				return core::Ok();
			}
		}

		template<typename Arc>
		Result serialize(Arc&& ar) {
			if constexpr(std::is_convertible_v<T&&, core::Function<Result(Arc&)>>) {
				return _t(ar);
			} else {
				return ar(_t());
			}
		}


	private:
		T _t;
};

template<typename T>
class Condition : Function<T> {
	public:
		Condition(bool c, T&& t) : Function<T>(y_fwd(t)), _c(c) {
		}

		template<typename Arc>
		Result deserialize(Arc&& ar) {
			if(_c) {
				return Function<T>::deserialize(ar);
			}
			return core::Ok();
		}

		template<typename Arc>
		Result serialize(Arc&& ar) {
			if(_c) {
				return Function<T>::serialize(ar);
			}
			return core::Ok();
		}


	private:
		bool _c;
};

template<typename T>
class Array {
	public:
		Array(usize s, T&& t) : _s(s), _t(y_fwd(t)) {
		}

		template<typename Arc>
		Result deserialize(Arc&& ar) {
			return ar.array(_t, _s);
		}

		template<typename Arc>
		Result serialize(Arc&& ar) {
			return ar.array(_t, _s);
		}


	private:
		usize _s;
		T _t;
};

}

template<typename... Args>
detail::Checker<Args...> check(Args&&... args) {
	return detail::Checker<Args...>(y_fwd(args)...);
}

template<typename T>
detail::Function<T> func(T&& t) {
	return detail::Function<T>(y_fwd(t));
}

template<typename T>
detail::Condition<T> cond(bool c, T&& t) {
	return detail::Condition<T>(c, y_fwd(t));
}

template<typename T>
detail::Array<T> array(usize s, T&& t) {
	return detail::Array<T>(s, y_fwd(t));
}

}
}

#endif // Y_SERDE2_SERDE_H
