#pragma once 

#include <polar/asset/base.h>

namespace polar::asset {
	struct uniforms : base {
	};

	inline serializer &operator<<(serializer &s, const uniforms &asset) {
		return s;
	}

	inline deserializer &operator>>(deserializer &s, uniforms &asset) {
		return s;
	}

	template<> inline std::string name<uniforms>() { return "uniforms"; }
}
