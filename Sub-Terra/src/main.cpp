#include "common.h"
#include "Freefall.h"

int main(int argc, char **argv) {
	std::vector<std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.emplace_back(std::string(argv[i]));
	}

	Freefall().Run(args);
	return 0;
}
