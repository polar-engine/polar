#pragma once

#include <polar/component/sprite/base.h>

namespace polar { namespace component { namespace sprite {
	class box : public base {
	public:
		void render_me() override final {
			const int width = 12;
			const int height = 12;

			SDL(surface = SDL_CreateRGBSurface(0, width, height, 32, 0xff, 0xff00, 0xff0000, 0xff000000));
			SDL(SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0)));
			SDL_Rect rect;
			rect.x = 1;
			rect.y = 1;
			rect.w = width - 2;
			rect.h = height - 2;
			SDL(SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 255, 255)));
		}
	};
} } }
