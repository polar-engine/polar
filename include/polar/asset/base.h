#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>
#include <polar/core/serializer.h>
#include <polar/util/endian.h>

namespace polar { namespace asset {
	using core::serializer;
	using core::deserializer;

	struct base {
		virtual ~base() {}
	};

	template<typename T> inline std::string name() = delete;
} }
