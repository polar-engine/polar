//#include <string>

#include "SubTerra.h"

int main(int argc, char **argv) {
	std::vector<std::string const> args;
	for (int i = 0; i < argc; ++i) {
		args.emplace_back(std::string(argv[i]));
	}

	SubTerra().Run(args);
}
