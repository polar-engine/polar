#pragma once

#include "Sprite.h"

class SliderSprite : public Sprite {
private:
	const float width;
	const float height;
	const float alpha;
public:
	SliderSprite(const Point2 pos, const float width, const float height, const float alpha) : Sprite(pos), width(width), height(height), alpha(alpha) {}

	void RenderMe() override final {
		const float edgeSize = 2;
		const float edgePadding = 2;
		const float boxSize = height;

		SDL(surface = SDL_CreateRGBSurface(0, width, height, 32, 0xff, 0xff00, 0xff0000, 0xff000000));

		SDL_Rect rect;

		// edges
		rect.y = 0;
		rect.h = height;

		// left edge
		rect.x = 0;
		rect.w = edgeSize;
		SDL(SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 255, 255)));

		// right edge
		rect.x = width - edgeSize;
		rect.w = edgeSize;
		SDL(SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 255, 255)));

		// box
		float min = edgeSize + edgePadding;
		float max = width - min - boxSize;
		rect.y = 0;
		rect.h = boxSize;
		rect.x = glm::mix(min, max, alpha);
		rect.w = boxSize;
		SDL(SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 0)));
		rect.y += 2;
		rect.h -= 4;
		rect.x += 2;
		rect.w -= 4;
		SDL(SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 77, 204, 255)));
	}
};
