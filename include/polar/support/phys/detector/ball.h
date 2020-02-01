#pragma once

#include <polar/math/types.h>
#include <polar/support/phys/detector/base.h>

namespace polar::support::phys::detector {
	class ball : public base {
	  public:
		ball() = default;
		ball(math::decimal radius) : base(math::point3(radius)) {}
	};
} // namespace polar::support::phys::detector
