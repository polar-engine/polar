#pragma once

#include <polar/component/base.h>
#include <polar/support/phys/boundingbox.h>

namespace polar {
namespace component {
	class bounds : public base {
	  public:
		support::phys::boundingbox box;
		bounds() {}
		bounds(const Point3 &position, const Point3 &size,
		       const bool &skipRoot = false)
		    : box(position, size, skipRoot) {}
	};
}
}
