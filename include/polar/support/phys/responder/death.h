#pragma once

#include <polar/support/phys/responder/base.h>

namespace polar::support::phys::responder {
	class death : public base {
	  public:
		void respond(core::polar &engine, core::weak_ref object, DeltaTicks) override {
			engine.remove(object);
		}
	};
} // namespace polar::support::phys::responder
