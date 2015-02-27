#pragma once

#include <vector>
#include "Asset.h"

template<typename T> struct ListAsset : Asset {
	std::vector<T> elements;

	ListAsset() {}
};

template<typename T> inline std::ostream & operator<<(std::ostream &os, ListAsset<T> asset) {
	const uint32_t length = swapbe(static_cast<uint32_t>(asset.elements.size()));
	os << std::string(reinterpret_cast<const char *>(&length), 4);
	for(auto &x : asset.elements) {
		os << x;
	}
	return os;
}

template<typename T> inline std::istream & operator>>(std::istream &is, ListAsset<T> &asset) {
	uint32_t length;
	is.read(reinterpret_cast<char *>(&length), 4);
	length = swapbe(length);

	for(uint32_t i = 0; i < length; ++i) {
		T x;
		is >> x;
		asset.elements.emplace_back(x);
	}

	return is;
}
