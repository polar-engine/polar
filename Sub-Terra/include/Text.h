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
		SDL(surface = TTF_RenderUTF8_Blended(asset->font, str.data(), { 255, 255, 255, 255 }));
	}

	void SetText(const std::string str) {
		this->str = str;
		Render();
	}
};
