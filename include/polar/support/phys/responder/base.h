#pragma once

#include <polar/core/polar.h>

namespace polar::support::phys::responder {
	class base {
	  public:
		virtual ~base() = default;
		virtual void respond(core::polar &, core::weak_ref, DeltaTicks) {}
	};
} // namespace polar::support::phys::responder
