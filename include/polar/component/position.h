#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class position : public base {
		template<typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<Point3> pos;
		position(const Point3 pos = Point3(0)) : pos(pos) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->pos);
		}
	};
} // namespace polar::component
