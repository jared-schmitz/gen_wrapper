#include <stddef.h>
#include <utility>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#include "command_buffer.h"
#include "tuple_utils.h"
#include "libcpp-util/cxx14/array_ref.h"

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

namespace {
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

bool convert_args(array_ref<string_ref>) {
	return true;
}

// Recurse and convert each argument in turn
template <typename T, typename... Ts>
bool convert_args(array_ref<string_ref> command, T& arg, Ts&&... args) {
	bool success = convert(command.front().data(), arg);
	command.pop_front();
	return success && convert_args(command, std::forward<Ts>(args)...);
}
template <typename Tuple, std::size_t... I>
bool convert_all_(array_ref<string_ref> command, Tuple& t,
		std::index_sequence<I...>) {
	return convert_args(command, std::get<I>(t)...);
}

// Splat a tuple of arguments to convert
template <typename Tuple>
bool convert_all(array_ref<string_ref> command, Tuple& t) {
	return convert_all_(command, t,
			std::make_index_sequence<my_tuple_size<Tuple>::value>());
}

bool tokenize(std::string& line, std::vector<string_ref>& args) {
	size_t end_of_command = line.find_first_of(" \t;");
	line[end_of_command] = '\0';
	size_t end_of_arg = end_of_command;
	while (1) {
		size_t begin_of_arg = end_of_arg + 1;
		end_of_arg = line.find_first_of(" \t;", begin_of_arg);

		// Skip empty tokens
		if (begin_of_arg == end_of_arg)
			continue;

		// Don't explode at the end of the string...
		if (end_of_arg == std::string::npos)
			end_of_arg = line.size();

		// Grab the argument
		string_ref this_arg = string_ref(line.data() + begin_of_arg,
				end_of_arg - begin_of_arg);
		args.push_back(this_arg);

		// If we're at the end break out
		if (end_of_arg == line.size())
			break;
		// If we're at a semi-colon, that's the end of this
		// command. FIXME: Really should be handled at a higher
		// level because this is two commands...
		if (line[end_of_arg] == ';') {
			line[end_of_arg] = '\0';
			break;
		}
		// Parsing later requires null-terminated strings
		line[end_of_arg] = '\0';
	}
	return true; // TODO: Define some parsing errors :)
}
}

class function_mapping {
public:
	typedef std::function<void(array_ref<string_ref>)> func_type;
private:
	// Returns a lambda which captures a pointer to the function. The lambda
	// converts all the arguments from strings to the appropriate parameter
	// types. It will issue (TODO: in a manner to be decided) an error if
	// conversion fails. Otherwise, the function is invoked.
	template <typename F, typename... Args>
	func_type generate_wrapper(F&& f, std::string usage)
	{
		return [f, usage](array_ref<string_ref> string_args) {
			// args is a tuple with its args removed of references
			// and const/volatile
			using value_tuple = 
				typename tuple_valify<Args...>::type;
			value_tuple args;
			constexpr size_t arg_size = sizeof...(Args);
			if (arg_size != string_args.size()) {
				fprintf(stderr, "Incorrect arity, expected %zu, got %zu\n", arg_size,
						string_args.size());
				if (!usage.empty())
					fprintf(stderr, "usage: %s\n", usage.data());
				return;
			}
			bool success = convert_all(string_args, args);
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
	void add_mapping(const std::string& command, R (*func)(Args...),
			std::string usage) {
		typedef decltype(func) target_type;
		mappings.emplace(command,
				generate_wrapper<target_type,
				Args...>(std::forward<target_type>(func), usage));
	}

	template <class R, class... Args>
	void add_mapping(const std::string& command, R (*func)(Args...)) {
		typedef decltype(func) target_type;
		mappings.emplace(command,
				generate_wrapper<target_type,
				Args...>(std::forward<target_type>(func), ""));

	}

	void execute_command(std::string c) {
		// Parse out the command
		c.push_back('\0');
		size_t end_of_command = c.find_first_of(" \t;");

		auto i = mappings.find(string_ref(c.data(),
					end_of_command).str());
		if (i == mappings.end()) {
			fprintf(stderr, "No command \"%s\"\n", c.data());
			return;
		}
		std::vector<string_ref> string_args;
		tokenize(c, string_args);

		fprintf(stderr, "Executing \"%s\"\n", c.data());
		i->second(string_args);
	}
};
