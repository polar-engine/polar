#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class playercamera : public base {
		template<typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<math::point3> distance;
		integrable<math::point3> position;
		integrable<math::quat> orientation;

		playercamera(const math::point3 &distance = math::point3(0, 0, 0),
		             const math::point3 &position = math::point3(0, 0, 0),
		             const math::point3 &euler    = math::point3(0, 0, 0))
		    : distance(distance), position(position),
		      orientation(math::point3(euler)) {
			add<property::integrable>();
			auto prop = get<property::integrable>().lock();
			if(prop) {
				prop->add(&this->distance);
				prop->add(&this->position);
				prop->add(&this->orientation);
			}
		}

		virtual std::string name() const override { return "playercamera"; }
	};
} // namespace polar::component
