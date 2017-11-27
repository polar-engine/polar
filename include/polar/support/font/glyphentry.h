#pragma once

#include <polar/core/types.h>
#include <polar/util/sdl.h>

namespace polar::support::font {
	struct glyphentry {
		bool active          = false;
		int origin           = 0;
		Point2i min          = Point2i(0);
		Point2i max          = Point2i(0);
		int advance          = 0;
		SDL_Surface *surface = nullptr;
	};
} // namespace polar::support::font
