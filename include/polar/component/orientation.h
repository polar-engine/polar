#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class orientation : public base {
	  public:
		Quat orient{1, 0, 0, 0};

		orientation() {}
		orientation(const Quat &orient) : orient(orient) {}
		orientation(const Quat &&orient) : orient(orient) {}
		orientation(const Point3 &euler) : orient(euler) {}
		orientation(const Point3 &&euler) : orient(euler) {}
	};
} // namespace polar::component
