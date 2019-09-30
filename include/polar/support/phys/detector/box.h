#pragma once

#include <polar/core/types.h>
#include <polar/support/phys/detector/base.h>

namespace polar::support::phys::detector {
	class box : public base {
	  public:
		Point3 size{1};

		box() = default;
		box(Point3 size) : size(size) {}
	};
} // namespace polar::support::phys::detector
