#pragma once

#include "Asset.h"

class TextAsset : public Asset {
public:
	const uint64_t length;
	const std::string text;

	static std::string Type() { return "text"; }
	static TextAsset Load(const std::string &data) {
		const uint64_t dataLength = data.length();
		if(dataLength < 8) { throw std::runtime_error("missing data length"); }
		const uint64_t length = hton64(*reinterpret_cast<const uint64_t *>(data.c_str()));
		if(length > dataLength - 8) { throw std::runtime_error("invalid data length"); }
		return TextAsset(length, data.substr(8, static_cast<unsigned int>(length)));
	}
	TextAsset(const uint64_t length, const std::string &text) : Asset("text"), length(length), text(text) {}
	std::string Save() const override final {
		const uint64_t beLength = ntoh64(length);
		return std::string(reinterpret_cast<const char *>(&beLength), 8) + text;
	}
};
