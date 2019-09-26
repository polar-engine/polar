#pragma once

#include <glm/gtx/rotate_vector.hpp>
#include <random>

namespace polar::math {
	class random {
		std::mt19937_64 engine;
	  public:
		random(uint64_t seed = 0) {
			engine.seed(seed);
		}

		inline explicit operator uint64_t() {
			return engine();
		}

		inline explicit operator double() {
			uint64_t mantissa = uint64_t(*this);
			uint64_t exponent = 0x3ff; // 2 ^ (1023 - 1023) = 2 ^ 0
			uint64_t d = (mantissa >> 12) | (exponent << 52);
			return *reinterpret_cast<double *>(&d) - 1;
		}

		inline explicit operator Decimal() {
			return Decimal(double(*this));
		}

		inline Decimal linear(Decimal min = 0, Decimal max = 1) {
			auto range = max - min;
			return Decimal(*this) * range + min;
		}

		inline Point2 disc(Decimal radius) {
			auto length = glm::sqrt(Decimal(*this)) * radius;
			auto angle = linear(0, 3.14159265358979f * 2);
			auto p = Point2(0, length);
			return glm::rotate(p, angle);
		}

		inline Point3 ball(Decimal radius) {
			auto length = glm::sqrt(Decimal(*this)) * radius;
			auto yaw   = linear(0, 3.14159265358979f * 2);
			auto pitch = linear(-3.14159265358979f, 3.14159265358979f);
			auto p = Point3(0, 0,  -length);
			auto yp = glm::rotate(p, yaw, Point3(0, 1, 0));
			return glm::rotate(yp, pitch, Point3(1, 0, 0));
		}
	};
} // nammespace polar::math
