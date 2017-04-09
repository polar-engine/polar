#pragma once

#include "Component.h"
#include "FontAsset.h"

class Text : public Component {
protected:
	FontAsset font;
	std::string str;
public:
	Text(const FontAsset &font, const std::string &str) : font(font), str(str) {}
};
