#pragma once

namespace polar::support::shader {
	enum class outputtype : uint8_t {
		invalid = 0,
		depth   = 1,
		rgb8    = 2,
		rgba8   = 3,
		rgb16f  = 4,
		rgba16f = 5,
		rgb32f  = 6,
		rgba32f = 7
	};

	enum class shadertype : uint8_t { invalid = 0, vertex = 1, fragment = 2 };
} // namespace polar::support::shader
