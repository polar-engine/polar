#pragma once

#include <polar/core/types.h>
#include <polar/support/phys/detector/base.h>

namespace polar::support::phys::detector {
	class ball : public base {
	  public:
		ball() = default;
		ball(Decimal radius) : base(Point3(radius)) {}
	};
} // namespace polar::support::phys::detector
