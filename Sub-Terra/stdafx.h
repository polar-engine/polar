#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <queue>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

#include <SDL/SDL.h>

/* SDL defines main to be SDL_main which is annoying */
#ifdef main
#undef main
#endif

static inline int _SDL_real(const char *file, const long line, const char *code) {
	const char *err = SDL_GetError();
	if(strlen(err) > 0) {
		//ERR("SDL: %s\n", err);
		//DEBUG("  %s:%ld\n", file, line);
		//DEBUG("    %s\n\n", code);
	}
	return strlen(err) == 0;
}

#define SDL(CODE) ((CODE), _SDL_real(__FILE__, __LINE__, #CODE))
