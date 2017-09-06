#pragma once

#include <array>
#include <polar/util/gl.h>

namespace polar { namespace support { namespace gl32 {
	struct fontcache_entry {
		bool active = false;
		GLuint texture;
	};

	struct fontcache {
		std::array<fontcache_entry, 128> entries;
	};
} } }
