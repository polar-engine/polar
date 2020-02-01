#pragma once

#include <polar/math/point.h>
#include <polar/util/sdl.h>

namespace polar::support::font {
	struct glyphentry {
		bool active          = false;
		int origin           = 0;
		math::point2i min    = math::point2i(0);
		math::point2i max    = math::point2i(0);
		int advance          = 0;
		SDL_Surface *surface = nullptr;
	};
} // namespace polar::support::font
