#ifndef APPLY_H
#define APPLY_H

#include <tuple>
#include <utility>
#include <type_traits>

#include "sequences.h"

// Get tuple_size for references-to-tuple
template <typename T>
struct my_tuple_size :
	std::integral_constant<std::size_t,
	std::tuple_size<typename std::remove_reference<T>::type>::value> { };

template<typename F, typename Tuple, std::size_t... I>
auto apply_(F&& f, Tuple&& args, std::index_sequence<I...>) ->
decltype(std::forward<F>(f)(std::get<I>(std::forward<Tuple>(args))...)) {
	return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(args))...);
}

template<typename F, typename Tuple,
	typename Indices = std::make_index_sequence<my_tuple_size<Tuple>::value>>
auto apply(F&& f, Tuple&& args) ->
decltype(apply_(std::forward<F>(f), std::forward<Tuple>(args), Indices())) {
	return apply_(std::forward<F>(f), std::forward<Tuple>(args), Indices());
}
#endif
