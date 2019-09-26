#pragma once

#include <glm/gtx/rotate_vector.hpp>
#include <random>

namespace polar::math {
	class random {
		std::mt19937_64 engine;
	  public:
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

		inline Point2 disc(Decimal radius) {
			auto length = glm::sqrt(Decimal(*this)) * radius;
			auto angle = Decimal(*this) * 3.141592f * 2;
			auto p = Point2(0, length);
			return glm::rotate(p, angle);
		}
	};
} // nammespace polar::math
