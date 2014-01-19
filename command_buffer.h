#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include <string>
#include <vector>
#include <deque>
#include "string_ref.h"

class command {
private:
	std::string line;
	string_ref _cmd;
	std::vector<string_ref> args;
public:
	decltype(args)::iterator iterator;
	decltype(args)::const_iterator const_iterator;

	string_ref cmd() const { return _cmd; }

	iterator begin() { return args.begin(); }
	iterator end() { return args.end(); }
	const_iterator begin() const { return args.begin(); } 
	const_iterator end() const { return args.end(); } 

	size_t args_size() const {
		return args.size();
	}

	command(std::string cmd) : line(cmd) {
		// TODO: Tokenize?
	}
	~command() = default;
};

class command_buffer {
private:
	std::deque<command> commands;
public:
	void push_command(command cmd) {
		commands.emplace_back(cmd);
	}

	command pop_command() {
		command cmd = commands.front();
		commands.pop_front();
		return cmd;
	}
};
#endif
