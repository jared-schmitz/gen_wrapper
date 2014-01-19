#include "my_utility.h"
#include "command_buffer.h"

static command_buffer buffer;


static const char *pop_line() {
	static const char *lines[] = {
		"howdy 5 6.2",
		"no_such_command arg1",
		"kill jared 5",
	};

}
// Define a dummy pop_token to give things back to the framework
const char *pop_token() {
	static const char* foo[] = { "fuck", "5", "6.2" };
	static size_t i = 0;
	return foo[i++];
}

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
	fm.execute_command();
	buffer.push_command("howdy 5 6.2");
	buffer.push_command("no_such_command arg1");
	buffer.push_command("kill jared 5");
}
