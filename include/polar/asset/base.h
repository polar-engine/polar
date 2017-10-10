#pragma once

#include <polar/core/serializer.h>
#include <polar/util/endian.h>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>

namespace polar {
namespace asset {
	using core::serializer;
	using core::deserializer;

	struct base {
		virtual ~base() {}
	};

	template <typename T> inline std::string name() {
		static_assert(true,
		              "invalid template argument to polar::asset::name()");
		return "";
	}
}
}
