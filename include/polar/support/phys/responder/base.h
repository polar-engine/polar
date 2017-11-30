#pragma once

#include <polar/core/polar.h>

namespace polar::support::phys::responder {
	class base {
	  public:
		virtual ~base() = default;
		virtual void respond(core::polar *, IDType, DeltaTicks) {}
	};
} // namespace polar::support::phys::responder
