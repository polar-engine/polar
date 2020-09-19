#pragma once

#include <polar/asset/image.h>
#include <polar/component/base.h>

namespace polar::component {
	COMPONENT_BEGIN(texture)
		struct pixel {
			uint8_t r = 255;
			uint8_t g = 255;
			uint8_t b = 255;
			uint8_t a = 255;

			pixel() = default;
			pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

			inline uint8_t &operator[](const size_t index) {
				switch(index) {
				case 0:
					return r;
				case 1:
					return g;
				case 2:
					return b;
				case 3:
					return a;
				default:
					throw std::out_of_range("index must be no greater than 3");
				}
			}

			friend inline core::store_serializer &operator<<(core::store_serializer &s, const pixel &p) {
				return s << p.r << p.g << p.b << p.a;
			}

			friend inline core::store_deserializer &operator>>(core::store_deserializer &s, pixel &p) {
				return s >> p.r >> p.g >> p.b >> p.a;
			}
		};

		uint32_t width;
		uint32_t height;
		std::vector<pixel> pixels;

		texture() = default;
		texture(std::shared_ptr<asset::image> as) : width(as->width), height(as->height) {
			for(auto &p : as->pixels) {
				pixels.emplace_back(p.red, p.green, p.blue, p.alpha);
			}
		}

		bool serialize(core::store_serializer &s) const override {
			s << width << height << pixels;
			return true;
		}

		static std::shared_ptr<texture> deserialize(core::store_deserializer &s) {
			auto c = std::make_shared<texture>();
			s >> c->width >> c->height >> c->pixels;
			return c;
		}
	COMPONENT_END(texture, texture)
} // namespace polar::component
