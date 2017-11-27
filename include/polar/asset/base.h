#pragma once

#include <polar/core/serializer.h>
#include <polar/util/endian.h>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>

namespace polar::asset {
	using core::deserializer;
	using core::serializer;

	struct base {
		virtual ~base() {}
	};

	template<typename T> inline std::string name() {
		static_assert(true,
		              "invalid template argument to polar::asset::name()");
		return "";
	}
} // namespace polar::asset
