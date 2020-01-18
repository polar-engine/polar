#pragma once

#include <polar/core/deltaticks.h>

namespace polar::support::sched::clock {
	class base {
	  private:
		DeltaTicks accumulator = 0;
	  public:
		double frequency = 50;

		virtual ~base() {}

		auto timestep() const {
			return DeltaTicks(DeltaTicksBase::rep(ENGINE_TICKS_PER_SECOND / frequency));
		}

		auto delta() const {
			return accumulator.Seconds();
		}

		void accumulate(DeltaTicks dt) {
			accumulator += dt;
			if(accumulator.Seconds() > 1.0f) { accumulator.SetSeconds(1.0f); }
		}

		bool tick() {
			auto ts = timestep();
			if(accumulator >= ts) {
				accumulator -= ts;
				return true;
			} else {
				return false;
			}
		}
	};
} // namespace polar::support::sched::clock
