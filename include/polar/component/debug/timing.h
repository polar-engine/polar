#pragma once

#include <polar/component/base.h>

namespace polar::component::debug {
	class timing : public base {
	  public:
		virtual std::string name() const override { return "debug_timing"; }
	};
} // namespace polar::component::debug
