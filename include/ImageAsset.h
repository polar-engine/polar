#pragma once

#include "Serializer.h"
#include "Asset.h"

struct ImagePixel : Asset {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

inline Serializer & operator<<(Serializer &s, ImagePixel pixel) {
	return s << pixel.red << pixel.green << pixel.blue << pixel.alpha;
}

inline Deserializer & operator>>(Deserializer &s, ImagePixel &pixel) {
	return s >> pixel.red >> pixel.green >> pixel.blue >> pixel.alpha;
}

struct ImageAsset : Asset {
	uint32_t width;
	uint32_t height;
	std::vector<ImagePixel> pixels;
};

inline Serializer & operator<<(Serializer &s, ImageAsset asset) {
	return s << asset.width << asset.height << asset.pixels;
}

inline Deserializer & operator>>(Deserializer &s, ImageAsset &asset) {
	return s >> asset.width >> asset.height >> asset.pixels;
}

template<> inline std::string AssetName<ImageAsset>() { return "Image"; }
