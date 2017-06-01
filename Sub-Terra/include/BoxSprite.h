#pragma once

#include "Sprite.h"

class BoxSprite : public Sprite {
public:
	BoxSprite(const Point2 scale = Point2(1), const Point2 pos = Point2(0), const Point4 color = Point4(1), const Origin origin = Origin::BottomLeft)
		: Sprite(pos, origin, color, scale) {}

	void RenderMe() override final {
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
