#pragma once

#include <string>
#include "Asset.h"

template<> inline std::string AssetName<std::string>() { return "Text"; }
