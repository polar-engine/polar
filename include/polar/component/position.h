#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar { namespace component {
	class position : public base {
		template<typename T> using integrable = support::integrator::integrable<T>;
	public:
		integrable<Point3> pos;
		position() : pos(Point3(0, 0, 0)) {}
		position(const Point3 pos) : pos(pos) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->pos);
		}
	};
} }
