#pragma once

#include "SDL/SDL.h"

/* SDL defines main to be SDL_main which is annoying */
#ifdef main
#undef main
#endif

inline bool _SDL_real(const char *file, const long line, const char *code) {
	const char *err = SDL_GetError();
	if(err[0] != '\0') {
		ENGINE_DEBUGERROR("SDL: 0x" << err,
						  "    " << file << ' ' << line << '\n' <<
						  "    " << code);
	}
	return err[0] == '\0';
}

#define ENGINE_SDL(CODE) ((CODE), _SDL_real(BASEFILE, __LINE__, #CODE))
#define SDL ENGINE_SDL
