#ifndef TUPLE_VALIFY_H
#define TUPLE_VALIFY_H

#include <tuple>
#include <type_traits>
#include "sequences.h"

template <typename... Ts>
struct tuple_valify {
	typedef std::tuple<typename std::decay<Ts>::type...> type;
};

namespace {
// Get tuple_size for references-to-tuple
template <typename T>
struct my_tuple_size :
	std::integral_constant<std::size_t,
	std::tuple_size<typename std::remove_reference<T>::type>::value> { };

// Call 'f' with tuple as arguments, expanding std::get pattern.
template<typename F, typename Tuple, std::size_t... I>
auto apply_(F&& f, Tuple&& args, std::index_sequence<I...>) ->
decltype(std::forward<F>(f)(std::get<I>(std::forward<Tuple>(args))...)) {
	return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(args))...);
}
}

// Call 'f' with tuple as arguments
template<typename F, typename Tuple,
	typename Indices = std::make_index_sequence<my_tuple_size<Tuple>::value>>
auto apply(F&& f, Tuple&& args) ->
decltype(apply_(std::forward<F>(f), std::forward<Tuple>(args), Indices())) {
	return apply_(std::forward<F>(f), std::forward<Tuple>(args), Indices());
}
#endif
