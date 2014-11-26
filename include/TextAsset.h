#pragma once

#include "Asset.h"

class TextAsset : public Asset {
public:
	const uint32_t length;
	const std::string text;

	static std::string Type() { return "text"; }
	TextAsset(const uint32_t length, const std::string &text) : Asset("text"), length(length), text(text) {}
	static TextAsset Load(const std::string &data) {
		const uint32_t dataLength = static_cast<uint32_t>(data.length());
		if(dataLength < 4) { ENGINE_THROW("missing data length"); }
		const uint32_t length = swapbe(*reinterpret_cast<const uint32_t *>(data.c_str()));
		if(length > dataLength - 4) { ENGINE_THROW("invalid data length"); }
		return TextAsset(length, data.substr(4, static_cast<unsigned int>(length)));
	}
	std::string Save() const override final {
		const uint32_t beLength = swapbe(length);
		return std::string(reinterpret_cast<const char *>(&beLength), 4) + text;
	}
};
