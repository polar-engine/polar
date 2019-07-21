#pragma once

#include <polar/asset/vertex.h>

namespace polar::asset {
	struct triangle : base {
		vertex p;
		vertex q;
		vertex r;
	};

	inline serializer &operator<<(serializer &s, const triangle &asset) {
		return s << asset.p << asset.q << asset.r;
	}

	inline deserializer &operator>>(deserializer &s, triangle &asset) {
		return s >> asset.p >> asset.q >> asset.r;
	}

	template<> inline std::string name<triangle>() { return "triangle"; }
} // namespace polar::asset
