#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class ref : public base {
		core::ref _r;

	  public:
		ref(core::ref r) : _r(r) {}

		virtual std::string name() const override { return "ref"; }
	};
} // namespace polar::component
