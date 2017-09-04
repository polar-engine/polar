#pragma once

#include <polar/asset/base.h>

template<> inline std::string AssetName<std::string>() { return "Text"; }
