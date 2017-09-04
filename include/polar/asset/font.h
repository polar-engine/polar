#pragma once

#include <polar/asset/base.h>
#include <polar/util/sdl.h>

#include <SDL2/SDL_ttf.h>

/* SDL defines main to be SDL_main which is annoying */
#ifdef main
#undef main
#endif

struct GlyphEntry {
	bool active = false;
	int origin = 0;
	Point2i min = Point2i(0);
	Point2i max = Point2i(0);
	int advance = 0;
	SDL_Surface *surface = nullptr;
};

struct FontAsset : Asset {
	TTF_Font *font;
	int maxWidth = 0;
	int maxHeight = 0;
	int lineSkip = 0;
	std::array<GlyphEntry, 128> glyphs;
};

template<> inline std::string AssetName<FontAsset>() { return "Font"; }

inline Deserializer & operator>>(Deserializer &s, FontAsset &asset) {
	std::string data;
	s >> data;
	char *buffer = static_cast<char *>(malloc(data.size()));
	if(buffer == NULL) {
		DebugManager()->Fatal("failed to allocate memory for font data");
	} else {
		memcpy(buffer, data.data(), data.size());
	}

	SDL_RWops *rwopts;
	SDL(rwopts = SDL_RWFromConstMem(buffer, data.size()));
	SDL(asset.font = TTF_OpenFontRW(rwopts, false, 144));

	SDL(asset.lineSkip = TTF_FontLineSkip(asset.font));

	for(size_t i = 0; i < asset.glyphs.size(); ++i) {
		if(TTF_GlyphIsProvided(asset.font, i)) {
			auto &glyph = asset.glyphs[i];
			glyph.active = true;
			glyph.origin = asset.maxWidth;
			SDL(TTF_GlyphMetrics(asset.font, i, &glyph.min.x, &glyph.max.x, &glyph.min.y, &glyph.max.y, &glyph.advance));

			asset.maxWidth += glyph.max.x - glyph.min.x;
			int h = glyph.max.y - glyph.min.y;
			if(h > asset.maxHeight) { asset.maxHeight = h; }

			// render outline first for drop-shadow effect
			SDL(TTF_SetFontOutline(asset.font, 2));
			SDL(glyph.surface = TTF_RenderGlyph_Blended(asset.font, i, { 0, 0, 0, 255 }));

			// render foreground and blit on top of outline
			SDL(TTF_SetFontOutline(asset.font, 0));
			SDL_Surface *fg;
			SDL(fg = TTF_RenderGlyph_Blended(asset.font, i, { 255, 255, 255, 255 }));
			SDL(SDL_BlitSurface(fg, NULL, glyph.surface, NULL));
			SDL(SDL_FreeSurface(fg));
		}
	}

	return s;
}
