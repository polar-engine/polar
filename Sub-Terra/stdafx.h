#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

#include <SDL/SDL.h>

/* SDL defines main to be SDL_main which is annoying */
#ifdef main
#undef main
#endif

const char pathSeparator =
#ifdef _WIN32
'\\'
#else
'/'
#endif
;

inline const char *basename(const char *path) {
	const char *r = strrchr(path, pathSeparator);
	return r ? r + 1 : path;
}

#define BASEFILE (basename(__FILE__))

#define OUTPUT(S) (std::cout << S)
#define ERROR(S)  (std::cerr << "========== ERROR ==========\n" << S << '\n')

#ifdef _DEBUG
#define DEBUG(S) OUTPUT(S)
#define CONTINUE ((std::cout << "Press enter to continue."), (std::cin.ignore(1)))
#else
#define DEBUG(S)
#define CONTINUE
#endif

inline bool _SDL_real(const char *file, const long line, const char *code) {
	const char *err = SDL_GetError();
	if(err[0] != '\0') {
		ERROR("SDL: " << err);
		DEBUG("    " << file << ':' << line << '\n');
		DEBUG("    " << code << "\n\n");
	}
	return err[0] == '\0';
}

#define SDL(CODE) ((CODE), _SDL_real(BASEFILE, __LINE__, #CODE))
