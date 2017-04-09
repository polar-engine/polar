#pragma once

#include "Component.h"
#include "FontAsset.h"

class Text : public Component {
protected:
	FontAsset asset;
	std::string str;
public:
	SDL_Surface *surface;
	Point2 position;
	Point2 scale;

	Text(const FontAsset &asset, const std::string &str, const Point2 pos = Point2(0.0f), const Point2 scale = Point2(-1.0f))
		: asset(asset), str(str), position(pos), scale(scale) {
		SDL_Color color = { 255, 255, 0, 255 };
		SDL(this->surface = TTF_RenderUTF8_Blended(this->asset.font, this->str.data(), color));

		if(this->scale.x < 0.0f) { this->scale.x = surface->w; }
		if(this->scale.y < 0.0f) { this->scale.y = surface->h; }
	}
};
