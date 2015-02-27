#pragma once

#include <string>
#include "Asset.h"

struct TextAsset : Asset {
	std::string text;

	TextAsset() {}
	TextAsset(const char *text) : text(text) {}
	TextAsset(const std::string &text) : text(text) {}
	TextAsset(std::string &&text) : text(text) {}
};

inline std::ostream & operator<<(std::ostream &os, TextAsset asset) {
	const uint32_t length = swapbe(asset.text.length());
	os << std::string(reinterpret_cast<const char *>(&length), 4);
	os << asset.text;
	return os;
}

inline std::istream & operator>>(std::istream &is, TextAsset &asset) {
	uint32_t length;
	is.read(reinterpret_cast<char *>(&length), 4);
	length = swapbe(length);

	asset.text = std::string(length, '\0');
	is.read(&asset.text[0], length);

	return is;
}

template<> inline std::string AssetName<TextAsset>() { return "Text"; }
