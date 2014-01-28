#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include <string>
#include <vector>
#include <deque>
#include "libcpp-util/cxx14/string_ref.h"

class command_buffer {
private:
	std::deque<std::string> commands;
public:
	void push_command(const char *cmd) {
		commands.push_back(cmd);
	}
	void push_command(const std::string& cmd) {
		commands.push_back(cmd);
	}
	template <class... Args>
	void push_command(Args&&... args) {
		commands.emplace_back(std::forward<Args>(args)...);
	}

	std::string pop_command() {
		auto cmd = commands.front();
		commands.pop_front();
		return cmd;
	}
};
#endif
