#pragma once

#include <polar/component/base.h>
#include <polar/support/phys/boundingbox.h>

namespace polar::component {
	class bounds : public base {
	  public:
		support::phys::boundingbox box;
		bounds() {}
		bounds(const math::point3 &position, const math::point3 &size,
		       const bool &skipRoot = false)
		    : box(position, size, skipRoot) {}
	};
} // namespace polar::component
