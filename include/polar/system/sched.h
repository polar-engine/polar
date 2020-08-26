#pragma once

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <polar/support/sched/clock/base.h>
#include <polar/system/base.h>

namespace polar::system {
	class sched : public base {
	  public:
		using handler_type = std::function<void(DeltaTicks)>;

	  private:
		using clock_base = support::sched::clock::base;

		struct timer {
			clock_base clock;
			handler_type handler;
		};

		core::id nextID = 1;

		std::unordered_map<core::id, timer> timers;

	  protected:
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

			for(auto it = timers.begin(); it != timers.end();) {
				auto &timer = it->second;

				timer.clock.accumulate(dt);
				if(timer.clock.tick()) {
					timer.handler(timer.clock.timestep);
					it = timers.erase(it);
				} else {
					++it;
				}
			}
		}

	  public:
		static bool supported() { return true; }
		sched(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "sched"; }

		auto keep(core::ref r, math::decimal seconds) {
			auto id = nextID++;

			timers.emplace(id, timer{clock_base(seconds), [r] (auto) {}});

			auto sch = engine->get<sched>();
			return core::ref([sch, id] {
				if(auto ptr = sch.lock()) {
					ptr->timers.erase(id);
				}
			});
		}
	};
} // namespace polar::system
