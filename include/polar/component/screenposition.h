#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>
#include <polar/support/ui/origin.h>

namespace polar::component {
	class screenposition : public base {
		template<typename T> using integrable = support::integrator::integrable<T>;
		using origin_t                        = support::ui::origin;

	  public:
		integrable<math::point2> position;
		origin_t origin;

		screenposition(math::point2 position = math::point2(0), origin_t origin = origin_t::bottomleft)
		    : position(position), origin(origin) {
			add<property::integrable>();
			get<property::integrable>()->add(&this->position);
		}

		virtual std::string name() const override { return "screenposition"; }
	};
} // namespace polar::component
