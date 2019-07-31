#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class orientation : public base {
		template<typename... Ts>
		using integrable = support::integrator::integrable<Ts...>;
	  public:
		integrable<Quat, Point3> orient;

		orientation(const Quat orient = Quat{1, 0, 0, 0}) : orient(orient) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->orient);
		}

		orientation(const Point3 euler) : orientation(Quat(euler)) {}
	};
} // namespace polar::component
