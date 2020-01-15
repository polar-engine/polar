#include <polar/core/polar.h>
#include <polar/property/integrable.h>
#include <polar/system/event.h>
#include <polar/system/integrator.h>

namespace polar::system {
	void integrator::tick(DeltaTicks::seconds_type seconds) {
		for(auto it = engine->objects.left.begin();
		    it != engine->objects.left.end(); ++it) {
			auto property = it->info->get<property::integrable>().lock();
			if(property) {
				for(auto integrable : *property->get()) {
					integrable->integrate(seconds);
				}
			}
		}
		engine->get<event>().lock()->firein("integrator", "ticked", seconds);
	}

	void integrator::revert_by(size_t n) {
		if(n == 0) { return; }

		for(auto it = engine->objects.left.begin();
		    it != engine->objects.left.end(); ++it) {
			auto property = it->info->get<property::integrable>().lock();
			if(property) {
				for(auto integrable : *property->get()) {
					integrable->revert_by(n);
				}
			}
		}
	}

	void integrator::revert_to(size_t n) {
		for(auto it = engine->objects.left.begin();
		    it != engine->objects.left.end(); ++it) {
			auto property = it->info->get<property::integrable>().lock();
			if(property) {
				for(auto integrable : *property->get()) {
					integrable->revert_to(n);
				}
			}
		}
	}
}
