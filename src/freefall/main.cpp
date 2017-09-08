#include <freefall/app.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
	int argc = __argc;
	char **argv = __argv;
#else
int main(int argc, char **argv) {
#endif
	std::vector<std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.emplace_back(argv[i]);
	}

	polar::core::polar engine(args);
	auto app = freefall::app(engine);
	return 0;
}
