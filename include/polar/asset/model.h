#pragma once

#include <optional>
#include <polar/asset/triangle.h>

namespace polar::asset {
	struct model : base {
		std::vector<triangle> triangles;
		std::optional<std::string> material;
	};

	inline serializer &operator<<(serializer &s, const model &asset) {
		return s << asset.triangles << asset.material;
	}

	inline deserializer &operator>>(deserializer &s, model &asset) {
		return s >> asset.triangles >> asset.material;
	}

	template<> inline std::string name<model>() { return "model"; }
} // namespace polar::asset
