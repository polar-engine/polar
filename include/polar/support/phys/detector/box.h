#pragma once

#include <polar/math/types.h>
#include <polar/support/phys/detector/base.h>

namespace polar::support::phys::detector {
	class box : public base {
	  public:
		box() = default;
		box(math::point3 size) : base(size) {}
	};
} // namespace polar::support::phys::detector
