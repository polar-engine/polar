#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>
#include <polar/core/serializer.h>
#include <polar/util/endian.h>

struct Asset {
	virtual ~Asset() {}
};

template<typename T> inline std::string AssetName() {
	static_assert(true, "AssetName not specialized");
	return "";
}
