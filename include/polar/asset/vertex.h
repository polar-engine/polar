#pragma once

#include <polar/asset/base.h>

namespace polar::asset {
	struct vertex : base {
		Decimal x;
		Decimal y;
		Decimal z;
	};

	inline serializer &operator<<(serializer &s, const vertex &asset) {
		return s << asset.x << asset.y << asset.z;
	}

	inline deserializer &operator>>(deserializer &s, vertex &asset) {
		return s >> asset.x >> asset.y >> asset.z;
	}

	template<> inline std::string name<vertex>() { return "vertex"; }
} // namespace polar::asset
