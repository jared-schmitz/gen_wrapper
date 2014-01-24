#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include <string>
#include <vector>
#include <deque>
#include "libcpp-util/cxx14/string_ref.h"

class command {
private:
	bool _valid;
	std::string line;
	string_ref _cmd;
	std::vector<string_ref> args;

	static string_ref repoint_ref(const std::string& old_line,
			string_ref old_ref, const std::string& new_line) {
		size_t offset = old_ref.data() - old_line.data();
		return string_ref(new_line.data() + offset, old_ref.size());
	}

public:
	command(std::string cmd) : line(cmd) {
		// First nab the command, which is an identifier which starts
		// with an alphabetic character and then is solely alphanumeric
		// or an underscore.
		line.push_back('\0');
		size_t end_of_command = line.find_first_of(" \t;");
		_cmd = string_ref(line.data(), end_of_command);
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
		_valid = true;
	}
	typedef decltype(args)::iterator iterator;
	typedef decltype(args)::const_iterator const_iterator;

	string_ref cmd() const { return _cmd; }

	string_ref operator[](size_t idx) const {
		return args[idx];
	}

	iterator begin() { return args.begin(); }
	iterator end() { return args.end(); }
	const_iterator begin() const { return args.begin(); } 
	const_iterator end() const { return args.end(); } 

	size_t args_size() const {
		return args.size();
	}

	bool valid() const {
		return _valid;
	}

	command(const command& other)
		: _valid(other.valid()), line(other.line) {
		_cmd = repoint_ref(other.line, other._cmd, line);
		for (const auto& arg : other.args)
			args.push_back(repoint_ref(other.line, arg, line));
	}
	command& operator=(const command& other) {
		_valid = other.valid();
		line = other.line;
		_cmd = repoint_ref(other.line, other._cmd, line);
		for (const auto& arg : other.args)
			args.push_back(repoint_ref(other.line, arg, line));
		return *this;
	}
	~command() = default;
};

class command_buffer {
private:
	std::deque<command> commands;
public:
	template <class... Args>
	void push_command(Args&&... args) {
		commands.emplace_back(std::forward<Args>(args)...);
	}

	command pop_command() {
		command cmd = commands.front();
		commands.pop_front();
		return cmd;
	}
};
#endif
