#pragma once

#include "Component.h"
#include "FontAsset.h"

enum class Origin {
	BottomLeft,
	BottomRight,
	TopLeft,
	TopRight,
	Center
};

class Text : public Component {
protected:
	FontAsset asset;
	std::string str;
public:
	Point4 color;
	Origin origin;
	Point2 position;
	Point2 scale;
	SDL_Surface *surface;

	Text(const FontAsset &asset, const std::string &str, const Point2 pos = Point2(0.0f), const Origin origin = Origin::BottomLeft,
	     const Point4 color = Point4(1.0f), const Point2 scale = Point2(-1.0f))
		: asset(asset), str(str), position(pos), origin(origin), color(color), scale(scale) {
		Render();
	}

	void Render() {
		SDL(surface = TTF_RenderUTF8_Blended(asset.font, str.data(), { 255, 255, 255, 255 }));

		if(scale.x < 0.0f) { scale.x = surface->w; }
		if(scale.y < 0.0f) { scale.y = surface->h; }
	}
};
