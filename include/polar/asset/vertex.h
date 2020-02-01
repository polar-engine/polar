#pragma once

#include <polar/asset/base.h>
#include <polar/util/pack.h>

namespace polar::asset {
	struct vertex {
		math::point3 position = math::point3(0);
		math::point3 normal   = math::point3(0);
		math::point2 texcoord = math::point2(0);
	};

	inline serializer &operator<<(serializer &s, const vertex &asset) {
		return s << asset.position << asset.normal << asset.texcoord;
	}

	inline deserializer &operator>>(deserializer &s, vertex &asset) {
		return s >> asset.position >> asset.normal >> asset.texcoord;
	}
} // namespace polar::asset
