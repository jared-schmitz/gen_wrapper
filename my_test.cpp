#include "my_utility.h"
#include "command_buffer.h"

static command_buffer buffer;

int piss_off(int a, double b) {
	return printf("piss_off called with %d, %f\n", a, b);
}

void kill_player(const std::string& name) {
	printf("Killing player \"%s\"\n", name.c_str());
}

int main() {
	function_mapping fm;
	fm.add_mapping("howdy", std::function<int(int, double)>(piss_off));
//	fm.add_mapping("kill", std::function<void(const
//				std::string&)>(kill_player));
	buffer.push_command("howdy 5 6.2");
	buffer.push_command("no_such_command arg1");
	buffer.push_command("kill jared 5");
	fm.execute_command(buffer.pop_command());
	fm.execute_command(buffer.pop_command());
	fm.execute_command(buffer.pop_command());
}
