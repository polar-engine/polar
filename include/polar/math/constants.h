#pragma once

namespace polar::math {
	constexpr decimal PI                             = decimal(3.14159265358979323846264338327950288419716939937510);
	constexpr decimal TWO_PI                         = decimal(PI * 2.0);
	template<size_t DENOM> constexpr decimal PI_OVER = decimal(PI / DENOM);
} // namespace polar::math
