#pragma once

namespace polar::math {
	constexpr decimal PI                              = decimal(3.14159265358979323846264338327950288419716939937510);
	template<size_t FACTOR> constexpr decimal PI_BY   = decimal(PI * FACTOR);
	template<size_t DENOM>  constexpr decimal PI_OVER = decimal(PI / DENOM);
	constexpr decimal TWO_PI                          = PI_BY<2>;
} // namespace polar::math
