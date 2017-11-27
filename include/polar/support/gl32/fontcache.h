#pragma once

#include <array>
#include <polar/util/gl.h>

namespace polar::support::gl32 {
	struct fontcache_entry {
		bool active = false;
		GLuint texture;
	};

	struct fontcache {
		std::array<fontcache_entry, 128> entries;
	};
} // namespace polar::support::gl32
