#include <stddef.h>
#include <utility>
#include <functional>
#include <unordered_map>
#include <iostream>

#include "command_buffer.h"
#include "tuple_utils.h"

// TODO: How to better statically dispatch...
template <typename>
struct arg_converter;

#define DEFINE_FLOAT_CONVERTER(type, func) \
template <> struct arg_converter<type> { \
	static constexpr const char *type_name = "floating-point"; \
	static type function(const char *nptr, char **endptr) { \
		return func(nptr, endptr); \
	} \
}

#define DEFINE_INTEGRAL_CONVERTER(type, func) \
template <> struct arg_converter<type> { \
	static constexpr const char *type_name = "integer"; \
	static type function(const char *nptr, char **endptr) { \
		return func(nptr, endptr, 0); \
	} \
}

#define DEFINE_STRING_CONVERTER(type) \
template <> struct arg_converter<type> { \
	static constexpr const char *type_name = "string"; \
	static type function(const char *nptr, char **endptr) { \
		static char dummy = '\0'; \
		*endptr = &dummy; \
		return nptr; \
	} \
}

DEFINE_FLOAT_CONVERTER(double, strtod);
DEFINE_FLOAT_CONVERTER(float, strtof);
DEFINE_INTEGRAL_CONVERTER(unsigned, strtoul);
DEFINE_INTEGRAL_CONVERTER(unsigned long, strtoul);
DEFINE_INTEGRAL_CONVERTER(unsigned long long, strtoul);
DEFINE_INTEGRAL_CONVERTER(unsigned short, strtoul);
DEFINE_INTEGRAL_CONVERTER(int, strtol);
DEFINE_INTEGRAL_CONVERTER(long, strtol);
DEFINE_INTEGRAL_CONVERTER(long long, strtoll);
DEFINE_INTEGRAL_CONVERTER(short, strtol);
DEFINE_STRING_CONVERTER(string_ref);
DEFINE_STRING_CONVERTER(std::string);
DEFINE_STRING_CONVERTER(const char*);

#undef DEFINE_FLOAT_CONVERTER
#undef DEFINE_INTEGRAL_CONVERTER
#undef DEFINE_STRING_CONVERTER

template <typename T>
bool convert(const char* s, T& val) {
	typedef arg_converter<T> converter;
	char* err = NULL;
	val = converter::function(s, &err);
	if (*s != '\0' && *err == '\0') {
		return true;
	} else {
		fprintf(stderr, "Invalid arg \"%s\", expected %s\n", s,
				converter::type_name);
		return false;
	}
}

template <class Tuple, unsigned I, unsigned J>
struct Converter {
	static bool convert_all(const command& c, Tuple& Tup) {
		bool success = convert(c[I].data(), std::get<I>(Tup));
		success &= Converter<Tuple, I + 1, J>::convert_all(c, Tup);
		return success;
	}
};

template <class Tuple, unsigned I>
struct Converter<Tuple, I, I> {
	static bool convert_all(const command&, Tuple&) { return true; }
};

class function_mapping {
public:
	typedef std::function<void(const command&)> func_type;
private:
	// Returns a lambda which captures a pointer to the function. The lambda
	// converts all the arguments from strings to the appropriate parameter
	// types. It will issue (TODO: in a manner to be decided) an error if
	// conversion fails. Otherwise, the function is invoked.
	template <typename F, typename... Args>
	func_type generate_wrapper(F&& f, std::string usage)
	{
		return [f, usage](const command& c) {
			// args is a tuple with its args removed of references
			// and const/volatile
			using value_tuple = 
				typename tuple_valify<Args...>::type;
			value_tuple args;
			constexpr size_t arg_size = sizeof...(Args);
			if (arg_size != c.args_size()) {
				fprintf(stderr, "Incorrect arity, expected %zu, got %zu\n", arg_size,
						c.args_size());
				if (!usage.empty())
					fprintf(stderr, "usage: %s\n", usage.data());
				return;
			}
			bool success = Converter<value_tuple, 0, arg_size>::convert_all(c, args);
			// Only call the function if conversion succeeded.
			if (success)
				apply(f, args);
			else if (!usage.empty())
				fprintf(stderr, "usage: %s\n", usage.data());
		};
	}
	std::unordered_map<std::string, func_type> mappings;
public:
	// Maps a command string to a function of arbitrary type.
	template <class R, class... Args>
	void add_mapping(const std::string& command,
			std::function<R(Args...)>&& func, std::string usage) {
		typedef decltype(func) target_type;
		mappings.emplace(command,
				generate_wrapper<target_type,
				Args...>(std::forward<target_type>(func), usage));
	}

	template <class R, class... Args>
	void add_mapping(const std::string& command,
			std::function<R(Args...)>&& func) {
		typedef decltype(func) target_type;
		mappings.emplace(command,
				generate_wrapper<target_type,
				Args...>(std::forward<target_type>(func), ""));

	}

	void execute_command(const command& c) {
		auto i = mappings.find(c.cmd().str());
		if (i == mappings.end()) {
			fprintf(stderr, "No command \"%s\"\n", c.cmd().data());
			return;
		}
		fprintf(stderr, "Executing \"%s\"\n", c.cmd().data());
		i->second(c);
	}
};
