#include <polar/component/sprite/slider.h>
#include <polar/math/types.h>

namespace polar::component::sprite {
		void slider::render_me() {
			const int edgeSize    = 2;
			const int edgePadding = 2;
			const int boxSize     = int(height);
			const int w = int(width), h = int(height);

			SDL(surface = SDL_CreateRGBSurface(0, w, h, 32, 0xff, 0xff00,
			                                   0xff0000, 0xff000000));

			SDL_Rect rect;

			// edges
			rect.y = 0;
			rect.h = h;

			// left edge
			rect.x = 0;
			rect.w = edgeSize;
			SDL(SDL_FillRect(surface, &rect,
			                 SDL_MapRGB(surface->format, 255, 255, 255)));

			// right edge
			rect.x = w - edgeSize;
			rect.w = edgeSize;
			SDL(SDL_FillRect(surface, &rect,
			                 SDL_MapRGB(surface->format, 255, 255, 255)));

			// box
			int min = edgeSize + edgePadding;
			int max = w - min - boxSize;
			rect.y  = 0;
			rect.h  = boxSize;
			rect.x  = glm::mix(min, max, alpha);
			rect.w  = boxSize;
			SDL(SDL_FillRect(surface, &rect,
			                 SDL_MapRGB(surface->format, 0, 0, 0)));
			rect.y += 1;
			rect.h -= 2;
			rect.x += 1;
			rect.w -= 2;
			SDL(SDL_FillRect(surface, &rect,
			                 SDL_MapRGB(surface->format, 77, 204, 255)));
		}
}
