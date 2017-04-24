#pragma once

#include "Sprite.h"
#include "FontAsset.h"

class Text : public Sprite {
private:
	std::shared_ptr<FontAsset> asset;
	std::string str;
public:
	Text(std::shared_ptr<FontAsset> asset, const std::string str, const Point2 pos = Point2(0), const Origin origin = Origin::BottomLeft,
		const Point4 color = Point4(1), const Point2 scale = Point2(-1))
		: asset(asset), str(str), Sprite(pos, origin, color, scale) {}

	void RenderMe() override final {
		// we render the outline first to make a drop-shadow effect

		SDL(TTF_SetFontOutline(asset->font, 2));
		SDL(surface = TTF_RenderUTF8_Blended(asset->font, str.data(), { 0, 0, 0, 255 }));

		// then we render the foreground and blit it on top of the outline

		SDL(TTF_SetFontOutline(asset->font, 0));
		SDL_Surface *fg;
		SDL(fg = TTF_RenderUTF8_Blended(asset->font, str.data(), { 255, 255, 255, 255 }));
		SDL(SDL_BlitSurface(fg, NULL, surface, NULL));
		SDL(SDL_FreeSurface(fg));
	}

	void SetText(const std::string str) {
		this->str = str;
		Render();
	}
};
