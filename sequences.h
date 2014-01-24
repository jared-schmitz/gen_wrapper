#ifndef CXX14_SEQUENCES_H
#define CXX14_SEQUENCES_H

#include <cstddef>
namespace std {
template <typename T, T... Indices>
struct integer_sequence {};

template <size_t N, typename T, T... Ts>
struct build_sequence : build_sequence<N-1, T, N - 1, Ts...> {};

template <typename T, T... Ts>
struct build_sequence<0, T, Ts...> : integer_sequence<T, Ts...> {};

template <size_t... Ints>
using index_sequence = integer_sequence<size_t, Ints...>;

template <typename T, T N>
using make_integer_sequence = build_sequence<N, T>;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
}
#endif
