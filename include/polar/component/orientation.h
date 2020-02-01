#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class orientation : public base {
		template<typename... Ts>
		using integrable = support::integrator::integrable<Ts...>;
	  public:
		integrable<math::quat, math::point3> orient;

		orientation(const math::quat orient = math::quat{1, 0, 0, 0}) : orient(orient) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->orient);
		}

		orientation(const math::point3 euler) : orientation(math::quat(euler)) {}

		virtual std::string name() const override { return "orientation"; }
	};
} // namespace polar::component
