#pragma once

#include <polar/asset/base.h>

namespace polar::asset {
	template<size_t N>
	struct vertex_attrib {
		std::array<Decimal, N> components;

		inline Decimal &operator[](size_t i) {
			return components[i];
		}
	};

	template<size_t N>
	inline serializer &operator<<(serializer &s, const vertex_attrib<N> &attrib) {
		for(auto &c : attrib.components) {
			s << c;
		}
		return s;
	}

	template<size_t N>
	inline deserializer &operator>>(deserializer &s, vertex_attrib<N> &attrib) {
		for(auto &c : attrib.components) {
			s >> c;
		}
		return s;
	}

	struct vertex : base {
		vertex_attrib<3> position;
		vertex_attrib<3> normal;
		vertex_attrib<2> texcoord;
	};

	inline serializer &operator<<(serializer &s, const vertex &asset) {
		return s << asset.position << asset.normal << asset.texcoord;
	}

	inline deserializer &operator>>(deserializer &s, vertex &asset) {
		return s >> asset.position >> asset.normal >> asset.texcoord;
	}

	template<> inline std::string name<vertex>() { return "vertex"; }
} // namespace polar::asset
