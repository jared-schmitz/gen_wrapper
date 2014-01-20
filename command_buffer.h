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
public:
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

	command(std::string cmd) : line(cmd) {
		// First nab the command, which is an identifier which starts
		// with an alphabetic character and then is solely alphanumeric
		// or an underscore.
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
