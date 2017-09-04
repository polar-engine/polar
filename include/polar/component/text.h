#pragma once

#include <polar/component/base.h>
#include <polar/asset/font.h>

class Text : public Component {
public:
	std::shared_ptr<FontAsset> asset;
	std::string str;

	Text(std::shared_ptr<FontAsset> asset, std::string str) : asset(asset), str(str) {}
};
