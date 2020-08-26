#pragma once

#include <polar/component/clock/base.h>

namespace polar::component::clock {
	class simulation : public base {
	  public:
		std::string name() const override { return "clock_simulation"; }
	};
} // namespace polar::component::clock
