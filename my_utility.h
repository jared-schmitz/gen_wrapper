#include <stddef.h>
#include <utility>
#include <functional>
#include <unordered_map>
#include <iostream>

#include "apply.h"

const char* pop_token();

// TODO: How to better statically dispatch...
template <typename>
struct arg_converter;

template <> struct arg_converter<double> {
	static constexpr const char *type_name = "double";
	static double function(const char *nptr, char **endptr) {
		return strtod(nptr, endptr);
	}
};
template <> struct arg_converter<long double> {
	static constexpr const char *type_name = "long double";
	static long double function(const char *nptr, char **endptr) {
		return strtold(nptr, endptr);
	}
};
template <> struct arg_converter<float> {
	static constexpr const char *type_name = "float";
	static float function(const char *nptr, char **endptr) {
		return strtof(nptr, endptr);
	}
};
template <> struct arg_converter<long> {
	static constexpr const char *type_name = "long";
	static long function(const char *nptr, char **endptr) {
		return strtol(nptr, endptr, 0);
	}
};
template <> struct arg_converter<unsigned long> {
	static constexpr const char *type_name = "unsigned long";
	static unsigned long function(const char *nptr, char **endptr) {
		return strtoul(nptr, endptr, 0);
	}
};
template <> struct arg_converter<long long> {
	static constexpr const char *type_name = "long long";
	static long long function(const char *nptr, char **endptr) {
		return strtoll(nptr, endptr, 0);
	}
};
template <> struct arg_converter<unsigned long long> {
	static constexpr const char *type_name = "unsigned long long";
	static unsigned long long function(const char *nptr, char **endptr) {
		return strtol(nptr, endptr, 0);
	}
};
template <> struct arg_converter<int> {
	static constexpr const char *type_name = "int";
	static int function(const char *nptr, char **endptr) {
		return strtol(nptr, endptr, 0);
	}
};
template <> struct arg_converter<unsigned> {
	static constexpr const char *type_name = "unsigned int";
	static unsigned function(const char *nptr, char **endptr) {
		return strtoul(nptr, endptr, 0);
	}
};

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
	static bool convert_all(Tuple& Tup) {
		bool success = convert(pop_token(), std::get<I>(Tup));
		success &= Converter<Tuple, I + 1, J>::convert_all(Tup);
		return success;
	}
};

template <class Tuple, unsigned I>
struct Converter<Tuple, I, I> {
	static bool convert_all(Tuple&) { return true; }
};

class function_mapping {
private:
	// Returns a lambda which captures a pointer to the function. The lambda
	// converts all the arguments from strings to the appropriate parameter
	// types. It will issue (TODO: in a manner to be decided) an error if
	// conversion fails. Otherwise, the function is invoked.
	template <typename F, typename... Args>
	std::function<void(void)> generate_wrapper(F&& f)
	{
		return [f]() {
			std::tuple<Args...> args;
			bool success = Converter<std::tuple<Args...>,
				0,
			       	sizeof...(Args)>::convert_all(args);
			// Only call the function if conversion succeeded.
			if (success)
				apply(f, args);
		};
	}
	std::unordered_map<std::string, std::function<void(void)>> mappings;
public:
	// Maps a command string to a function of arbitrary type. The command 
	template <class R, class... Args>
	void add_mapping(const std::string& command,
			std::function<R(Args...)>&& func) {
		typedef decltype(func) func_type;
		mappings.emplace(command,
				generate_wrapper<func_type,
				Args...>(std::forward<func_type>(func)));		
	}

	void execute_command() {
		std::string s = pop_token();
		auto i = mappings.find(s);
		if (i == mappings.end()) {
			fprintf(stderr, "No command \"%s\"\n", s.c_str());
			return;
		}
		fprintf(stderr, "Executing \"%s\"\n", s.c_str());
		i->second();
	}
#if 0
	// FIXME: Args are unfortunately yanked by the converter functions,
	// which breaks 'one function, one responsibility'. So the below can't
	// be done.
	template <std::string... Ss>
	void execute_explicit_command(const char* command, Ss... args) {
		std::string s = pop_token();
		auto i = mappings.find(s);
		if (i == mappings.end()) {
			fprintf(stderr, "No command \"%s\"\n", s.c_str());
			return;
		}
		fprintf(stderr, "Executing \"%s\"\n", s.c_str());
		std::tuple<Args...> args;
		Converter<std::tuple<Args...>, 0, sizeof...(Args)>::convert_all(args);
		apply(f, args);
	}
#endif
};
