#pragma once 

#include <polar/asset/base.h>

namespace polar::asset {
	struct material : base {
		Point3 ambient = Point3(0);
		Point3 diffuse = Point3(1);
		Point3 specular = Point3(1);
		Decimal specular_exponent = 64;
	};

	inline serializer &operator<<(serializer &s, const material &asset) {
		return s << asset.ambient << asset.diffuse << asset.specular << asset.specular_exponent;
	}

	inline deserializer &operator>>(deserializer &s, material &asset) {
		return s >> asset.ambient >> asset.diffuse >> asset.specular >> asset.specular_exponent;
	}

	template<> inline std::string name<material>() { return "material"; }
}
