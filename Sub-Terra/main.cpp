#include "stdafx.h"
#include "SubTerra.h"

int main(int argc, char **argv) {
	std::vector<const std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.emplace_back(std::string(argv[i]));
	}

	SubTerra().Run(args);

	std::cout << "Press enter to continue.";
	std::cin.ignore(1);

	return 0;
}