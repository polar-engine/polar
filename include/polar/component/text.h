#pragma once

#include "FontAsset.h"

class Text : public Component {
public:
	std::shared_ptr<FontAsset> asset;
	std::string str;

	Text(std::shared_ptr<FontAsset> asset, std::string str) : asset(asset), str(str) {}
};
