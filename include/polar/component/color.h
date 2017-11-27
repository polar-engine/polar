#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class color : public base {
		template<typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<Point4> col;

		color(Point4 col = Point4(1)) : col(col) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->col);
		}
	};
} // namespace polar::component
