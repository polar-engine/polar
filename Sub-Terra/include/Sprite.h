#pragma once

#include "Component.h"
#include "sdl.h"

enum class Origin {
	BottomLeft,
	BottomRight,
	TopLeft,
	TopRight,
	Center
};

class Sprite : public Component {
public:
	Origin origin;
	Point2 position;
	Point2 scale;
	Point4 color;
	SDL_Surface *surface = nullptr;

	Sprite(const Point2 pos = Point2(0), const Origin origin = Origin::BottomLeft, const Point4 color = Point4(1), const Point2 scale = Point2(-1))
		: position(pos), origin(origin), color(color), scale(scale) {}

	virtual ~Sprite() {
		if(surface) { SDL(SDL_FreeSurface(surface)); }
	}

	virtual void Render() final {
		if(surface) { SDL(SDL_FreeSurface(surface)); }

		RenderMe();

		if(scale.x < 0.0f) { scale.x = static_cast<float>(surface->w); }
		if(scale.y < 0.0f) { scale.y = static_cast<float>(surface->h); }
	}

	virtual void RenderMe() = 0;
};
