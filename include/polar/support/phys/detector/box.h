#pragma once

#include <polar/core/types.h>
#include <polar/support/phys/detector/base.h>

namespace polar::support::phys::detector {
	class box : public base {
	  public:
		box() = default;
		box(Point3 size) : base(size) {}
	};
} // namespace polar::support::phys::detector
