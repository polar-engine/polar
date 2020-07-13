#include <polar/core/polar.h>
#include <polar/property/integrable.h>
#include <polar/system/event.h>
#include <polar/system/integrator.h>

namespace polar::system {
	void integrator::tick(DeltaTicks::seconds_type seconds) {
		auto &index = engine->objects.get<core::index::ref>();
		for(auto it = index.begin(); it != index.end(); ++it) {
			auto property = it->ptr->get<property::integrable>();
			if(property) {
				for(auto integrable : *property->get()) { integrable->integrate(seconds); }
			}
		}
		engine->get<event>().lock()->firein("integrator", "ticked", seconds);
	}

	void integrator::revert_by(size_t n) {
		if(n == 0) { return; }

		auto &index = engine->objects.get<core::index::ref>();
		for(auto it = index.begin(); it != index.end(); ++it) {
			auto property = it->ptr->get<property::integrable>();
			if(property) {
				for(auto integrable : *property->get()) { integrable->revert_by(n); }
			}
		}
	}

	void integrator::revert_to(size_t n) {
		auto &index = engine->objects.get<core::index::ref>();
		for(auto it = index.begin(); it != index.end(); ++it) {
			auto property = it->ptr->get<property::integrable>();
			if(property) {
				for(auto integrable : *property->get()) { integrable->revert_to(n); }
			}
		}
	}
} // namespace polar::system
