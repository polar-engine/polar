#pragma once

#include <polar/asset/triangle.h>

namespace polar::asset {
	struct model : base {
		std::vector<triangle> triangles;
	};

	inline serializer &operator<<(serializer &s, const model &asset) {
		return s << asset.triangles;
	}

	inline deserializer &operator>>(deserializer &s, model &asset) {
		return s >> asset.triangles;
	}

	template<> inline std::string name<model>() { return "model"; }
} // namespace polar::asset
