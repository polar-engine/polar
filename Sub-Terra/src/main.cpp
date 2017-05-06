#include "common.h"
#include "Freefall.h"

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
		args.emplace_back(std::string(argv[i]));
	}

#if defined(_WIN32)
	for(auto &arg : args) {
		if(arg == "-console") {
			AllocConsole();
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			std::wcout.clear();
			std::cout.clear();
			std::wcerr.clear();
			std::cerr.clear();
			std::wcin.clear();
			std::cin.clear();
			break;
		}
	}
#endif

	Freefall().Run(args);
	return 0;
}
