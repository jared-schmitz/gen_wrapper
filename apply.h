#ifndef APPLY_H
#define APPLY_H

#include <tuple>
#include <utility>
#include <type_traits>

namespace {
// Expand a tuple whose signature matches the argument signature of F, call F
// with the tuple's contents, and return the result.
template <unsigned K, class R, class F, class Tup>
struct Expander {
	template <class... Us>
	static R expand(F&& f, Tup&& t, Us&&... args) {
		return Expander<K - 1, R, F, Tup>::expand(
				f,
				std::forward<Tup>(t),
				std::get<K - 1>(std::forward<Tup>(t)),
				std::forward<Us>(args)...);
	}
};
template <class R, class F, class Tup>
struct Expander<0, R, F, Tup> {
	template <class... Us>
	static R expand(F&& f, Tup&&, Us&&... args) {
		return f(std::forward<Us>(args)...);
	}
};
}

// Helper to use Expander because it's hideous
template <class F, class... Ts>
auto apply(F&& f, const std::tuple<Ts...>& t)
	-> typename std::result_of<F(Ts...)>::type
{
	return Expander<sizeof...(Ts), 
	       typename std::result_of<F(Ts...)>::type,
	       F,
	       const std::tuple<Ts...>&>::expand(f, t);
}

#endif
