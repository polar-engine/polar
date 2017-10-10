#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar {
namespace component {
	class playercamera : public base {
		template <typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<Point3> distance;
		integrable<Point3> position;
		Quat orientation;
		playercamera(const Point3 &distance = Point3(0, 0, 0),
		             const Point3 &position = Point3(0, 0, 0),
		             const Point3 &euler    = Point3(0, 0, 0))
		    : distance(distance), position(position),
		      orientation(Point3(euler)) {
			add<property::integrable>();
			auto prop = get<property::integrable>().lock();
			if(prop) {
				prop->add(&this->distance);
				prop->add(&this->position);
			}
		}
	};
}
}
