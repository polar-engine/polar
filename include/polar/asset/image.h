#pragma once

#include <polar/asset/base.h>

namespace polar::asset {
	struct imagepixel {
		uint8_t red = 1.0;
		uint8_t green = 1.0;
		uint8_t blue = 1.0;
		uint8_t alpha = 1.0;

		inline uint8_t &operator[](const size_t index) {
			switch(index) {
			case 0:
				return red;
			case 1:
				return green;
			case 2:
				return blue;
			case 3:
				return alpha;
			default:
				throw std::out_of_range("index must be no greater than 3");
			}
		}
	};

	inline serializer &operator<<(serializer &s, const imagepixel &pixel) {
		return s << pixel.red << pixel.green << pixel.blue << pixel.alpha;
	}

	inline deserializer &operator>>(deserializer &s, imagepixel &pixel) {
		return s >> pixel.red >> pixel.green >> pixel.blue >> pixel.alpha;
	}

	struct image : base {
		uint32_t width;
		uint32_t height;
		std::vector<imagepixel> pixels;
	};

	inline serializer &operator<<(serializer &s, const image &asset) {
		return s << asset.width << asset.height << asset.pixels;
	}

	inline deserializer &operator>>(deserializer &s, image &asset) {
		return s >> asset.width >> asset.height >> asset.pixels;
	}

	template<> inline std::string name<image>() { return "image"; }
} // namespace polar::asset
