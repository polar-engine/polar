#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class scale : public base {
		template<typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<Point3> sc;

		scale(Point3 sc = Point3(1)) : sc(sc) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->sc);
		}

		virtual std::string name() const { return "scale"; }
	};
} // namespace polar::component
