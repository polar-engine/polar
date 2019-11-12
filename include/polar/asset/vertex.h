#pragma once

#include <polar/asset/base.h>
#include <polar/util/pack.h>

namespace polar::asset {
	struct vertex {
		Point3 position;
		Point3 normal;
		Point2 texcoord;
	};

	inline serializer &operator<<(serializer &s, const vertex &asset) {
		return s << asset.position << asset.normal << asset.texcoord;
	}

	inline deserializer &operator>>(deserializer &s, vertex &asset) {
		return s >> asset.position >> asset.normal >> asset.texcoord;
	}
} // namespace polar::asset
