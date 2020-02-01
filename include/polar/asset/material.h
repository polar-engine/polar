#pragma once 

#include <polar/asset/base.h>

namespace polar::asset {
	struct material : base {
		math::point3 ambient = math::point3(0);
		math::point3 diffuse = math::point3(1);
		math::point3 specular = math::point3(1);
		math::decimal specular_exponent = 64;
		std::optional<std::string> diffuse_map;
		std::optional<std::string> specular_map;
		std::optional<std::string> normal_map;
	};

	inline serializer &operator<<(serializer &s, const material &asset) {
		return s << asset.ambient << asset.diffuse << asset.specular << asset.specular_exponent << asset.diffuse_map << asset.specular_map << asset.normal_map;
	}

	inline deserializer &operator>>(deserializer &s, material &asset) {
		return s >> asset.ambient >> asset.diffuse >> asset.specular >> asset.specular_exponent >> asset.diffuse_map >> asset.specular_map >> asset.normal_map;
	}

	template<> inline std::string name<material>() { return "material"; }
}
