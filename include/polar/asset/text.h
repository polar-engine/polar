#pragma once

#include <polar/asset/base.h>

namespace polar::asset {
	template<> inline std::string name<std::string>() { return "text"; }
} // namespace polar::asset
