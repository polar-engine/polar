#pragma once

#include <polar/support/phys/responder/base.h>

namespace polar::support::phys::responder {
	class rigid : public base {
	  public:
		void respond(core::polar *engine, IDType id, DeltaTicks) override {
			if(auto p = engine->get<component::position>(id)) {
				p->pos.revert();
				p->pos.derivative()->x *= -1;
			}
		}
	};
} // namespace polar::support::phys::responder
