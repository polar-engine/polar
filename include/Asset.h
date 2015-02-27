#pragma once

#include <stdint.h>
#include <string>
#include <type_traits>
#include "endian.h"

struct Asset {
	virtual ~Asset() {}
};

template<typename T> inline std::string AssetName() {
	static_assert(false, "AssetName not specialized");
	return "";
}
