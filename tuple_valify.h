#ifndef TUPLE_VALIFY_H
#define TUPLE_VALIFY_H

#include <tuple>
#include <type_traits>

template <typename... Ts>
struct tuple_valify {
	typedef std::tuple<typename std::decay<Ts>::type...> type;
};
#endif
