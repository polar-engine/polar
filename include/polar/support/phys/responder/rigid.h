#pragma once

#include <polar/support/phys/responder/base.h>

namespace polar::support::phys::responder {
	class rigid : public base {
	  public:
		Point3 bounce{1};

		rigid(Point3 bounce) : bounce(bounce) {}

		void respond(core::polar *engine, IDType id, DeltaTicks) override {
			if(auto p = engine->get<component::position>(id)) {
				p->pos.revert();
				p->pos.derivative() *= -bounce;
			}
		}
	};
} // namespace polar::support::phys::responder
