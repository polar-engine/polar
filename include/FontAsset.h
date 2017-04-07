#pragma once

#include <string>
#include "Asset.h"

class FontAsset {};

template<> inline std::string AssetName<FontAsset>() { return "Font"; }
