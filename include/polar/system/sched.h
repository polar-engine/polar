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

		using binding_bimap = boost::bimap<
			boost::bimaps::set_of<core::id>,
			boost::bimaps::unordered_multiset_of<std::type_index>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<handler_type>
		>;
	  private:
		using clock_base = support::sched::clock::base;

		struct timer {
			clock_base clock;
			handler_type handler;
		};

		core::id nextID = 1;
		binding_bimap bindings;
		std::unordered_map<std::type_index, std::unique_ptr<clock_base>> clocks;

		std::unordered_map<core::id, timer> timers;
	  protected:
		void update(DeltaTicks &dt) override {
			for(auto &[ti, clock] : clocks) {
				clock->accumulate(dt);
				while(clock->tick()) {
					auto pairRight = bindings.right.equal_range(ti);
					for(auto it = pairRight.first; it != pairRight.second; ++it) {
						it->info(clock->timestep);
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

		template<
			typename Clock,
			typename = typename std::enable_if<std::is_base_of<clock_base, Clock>::value>::type
		> auto bind(handler_type handler) {
			std::type_index ti = typeid(Clock);
			auto id = nextID++;

			bindings.insert(binding_bimap::value_type(id, ti, handler));
			clocks.emplace(ti, std::make_unique<Clock>());

			auto sch = engine->get<sched>();
			return core::ref([sch, id] {
				if(auto ptr = sch.lock()) {
					ptr->bindings.left.erase(id);
				}
			});
		}

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

		template<
			typename Clock,
			typename = typename std::enable_if<std::is_base_of<clock_base, Clock>::value>::type
		> auto delta() {
			return clocks[typeid(Clock)]->delta();
		}
	};
} // namespace polar::system
