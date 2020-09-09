#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class scene : public base {
	  public:
		virtual std::string name() const override { return "scene"; }
	};
} // namespace polar::component
