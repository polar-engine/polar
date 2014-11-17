#pragma once

#include <SDL/SDL.h>

/* SDL defines main to be SDL_main which is annoying */
#ifdef main
#undef main
#endif

inline bool _SDL_real(const char *file, const long line, const char *code) {
	const char *err = SDL_GetError();
	if(err[0] != '\0') {
		ERROR("SDL: " << err);
		DEBUG("    " << file << ':' << line);
		DEBUG("    " << code << '\n');
	}
	return err[0] == '\0';
}

#define SDL(CODE) ((CODE), _SDL_real(BASEFILE, __LINE__, #CODE))
