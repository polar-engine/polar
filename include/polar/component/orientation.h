#pragma once

#include <polar/component/base.h>

namespace polar {
namespace component {
	class orientation : public base {
	  public:
		Quat orient;
		orientation() {}
		orientation(const Point3 &euler) : orient(euler) {}
		orientation(const Point3 &&euler) : orient(euler) {}
	};
}
}
