#include <iostream>
#include "FileSystem.h"

int main(int argc, char **argv) {
	auto files = FileSystem::Contents("assets");
	for(auto file : files) {
		std::cout << file;
	}
	std::cin.ignore(1);
	return 0;
}
