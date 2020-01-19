#pragma once

#include <polar/core/deltaticks.h>

namespace polar::support::sched::clock {
	class base {
	  private:
		DeltaTicks accumulator = 0;
	  public:
		DeltaTicks timestep;

		base(Decimal ts = Decimal(1) / Decimal(50)) {
			timestep.SetSeconds(ts);
		}

		virtual ~base() {}

		auto frequency() const {
			return Decimal(1) / timestep.Seconds();
		}

		auto delta() const {
			return accumulator.Seconds();
		}

		void accumulate(DeltaTicks dt) {
			accumulator += dt;

			auto max = timestep.Seconds() * 10;
			if(accumulator.Seconds() > max) { accumulator.SetSeconds(max); }
		}

		bool tick() {
			auto ts = timestep;
			if(accumulator >= ts) {
				accumulator -= ts;
				return true;
			} else {
				return false;
			}
		}
	};
} // namespace polar::support::sched::clock
