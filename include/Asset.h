#pragma once

#include <stdint.h>
#include <string>
#include <type_traits>
#include "endian.h"

struct Asset {
	virtual ~Asset() {}
};

template<typename T> inline std::string AssetName() {
	ENGINE_THROW("AssetName not specialized");
	return "";
}
