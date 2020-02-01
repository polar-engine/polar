#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class color : public base {
		template<typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<math::point4> col;

		color(math::point4 col = math::point4(1)) : col(col) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->col);
		}

		virtual std::string name() const override { return "color"; }
	};
} // namespace polar::component
