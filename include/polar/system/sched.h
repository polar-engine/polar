#pragma once

#include <polar/system/base.h>

namespace polar::system {
	class sched : public base {
	  private:
		void update(DeltaTicks &dt) override {
			std::map<component::clock::base *, std::vector<component::listener *>> clocks;

			auto ti_range = engine->objects.get<core::index::ti>().equal_range(typeid(component::listener));
			for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
				auto listener = static_cast<component::listener *>(ti_it->ptr.get());
				auto clock = engine->get<component::clock::base>(listener->ref());
				clocks[clock].emplace_back(listener);
			}

			for(auto &[clock, listeners] : clocks) {
				clock->accumulate(dt);
				while(clock->tick()) {
					for(auto &listener : listeners) {
						listener->trigger(clock->timestep);
					}
				}
			}
		}

	  public:
		static bool supported() { return true; }
		sched(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "sched"; }
	};
} // namespace polar::system
