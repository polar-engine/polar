#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>
#include <polar/support/ui/origin.h>

namespace polar {
namespace component {
	class screenposition : public base {
		template <typename T>
		using integrable = support::integrator::integrable<T>;
		using origin_t   = support::ui::origin;

	  public:
		integrable<Point2> position;
		origin_t origin;

		screenposition(Point2 position = Point2(0),
		               origin_t origin = origin_t::bottomleft)
		    : position(position), origin(origin) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->position);
		}
	};
}
}
