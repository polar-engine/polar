#pragma once

#include <polar/support/phys/detector/base.h>

namespace polar::support::phys::detector {
	class box : public base {
	  public:
		Point3 size{1};
	};
} // namespace polar::support::phys::detector
