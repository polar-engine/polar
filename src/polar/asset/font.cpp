#include <polar/asset/font.h>

namespace polar::asset {
	deserializer &operator>>(deserializer &s, font &asset) {
		std::string data;
		s >> data;
		char *buffer = static_cast<char *>(malloc(data.size()));
		if(buffer == nullptr) {
			log()->fatal("font", "failed to allocate memory for font data");
		} else {
			memcpy(buffer, data.data(), data.size());
		}

		SDL_RWops *rwopts;
		SDL(rwopts = SDL_RWFromConstMem(buffer, (int)data.size()));
		SDL(asset.ttf = TTF_OpenFontRW(rwopts, false, 144));

		SDL(asset.lineSkip = TTF_FontLineSkip(asset.ttf));

		for(Uint16 i = 0; i < asset.glyphs.size(); ++i) {
			if(TTF_GlyphIsProvided(asset.ttf, i)) {
				auto &glyph  = asset.glyphs[i];
				glyph.active = true;
				glyph.origin = asset.maxWidth;
				SDL(TTF_GlyphMetrics(asset.ttf, i, &glyph.min.x, &glyph.max.x,
				                     &glyph.min.y, &glyph.max.y,
				                     &glyph.advance));

				asset.maxWidth += glyph.max.x - glyph.min.x;
				int h = glyph.max.y - glyph.min.y;
				if(h > asset.maxHeight) { asset.maxHeight = h; }

				// render outline first for drop-shadow effect
				SDL(TTF_SetFontOutline(asset.ttf, 2));
				SDL(glyph.surface =
				        TTF_RenderGlyph_Blended(asset.ttf, i, {0, 0, 0, 255}));

				// render foreground and blit on top of outline
				SDL(TTF_SetFontOutline(asset.ttf, 0));
				SDL_Surface *fg;
				SDL(fg = TTF_RenderGlyph_Blended(asset.ttf, i,
				                                 {255, 255, 255, 255}));
				SDL(SDL_BlitSurface(fg, nullptr, glyph.surface, nullptr));
				SDL(SDL_FreeSurface(fg));
			}
		}

		return s;
	}
}
