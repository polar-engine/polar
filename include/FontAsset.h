#pragma once

#include <string>
#include "Asset.h"
#include "Serializer.h"
#include "sdl.h"

#include <SDL/SDL_ttf.h>

/* SDL defines main to be SDL_main which is annoying */
#ifdef main
#undef main
#endif

struct FontAsset : Asset {
	TTF_Font *font;
};

template<> inline std::string AssetName<FontAsset>() { return "Font"; }

inline Deserializer & operator>>(Deserializer &s, FontAsset &asset) {
	std::string data;
	s >> data;
	char *buffer = static_cast<char *>(malloc(data.size()));
	memcpy(buffer, data.data(), data.size());

	SDL_RWops *rwopts;
	SDL(rwopts = SDL_RWFromConstMem(buffer, data.size()));
	SDL(asset.font = TTF_OpenFontRW(rwopts, false, 36));

	return s;
}
