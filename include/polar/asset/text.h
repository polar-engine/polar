#pragma once

#include <polar/asset/base.h>

namespace polar {
namespace asset {
	template <> inline std::string name<std::string>() { return "text"; }
}
}
