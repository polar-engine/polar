#pragma once

#include <SDL_ttf.h>
#include <array>
#include <polar/asset/base.h>
#include <polar/support/font/glyphentry.h>
#include <polar/util/gl.h>
#include <polar/util/sdl.h>

namespace polar::asset {
	struct font : base {
		using glyphentry = support::font::glyphentry;

		TTF_Font *ttf;
		int maxWidth  = 0;
		int maxHeight = 0;
		int lineSkip  = 0;
		std::array<glyphentry, 128> glyphs;
	};

	deserializer &operator>>(deserializer &s, font &asset);

	template<> inline std::string name<font>() { return "font"; }
} // namespace polar::asset
