#pragma once

namespace polar::math {
	constexpr Decimal PI                             = Decimal(3.14159265358979323846264338327950288419716939937510);
	constexpr Decimal TWO_PI                         = Decimal(PI * 2.0);
	template<size_t DENOM> constexpr Decimal PI_OVER = Decimal(PI / DENOM);
} // namespace polar::math
