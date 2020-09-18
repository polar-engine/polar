#pragma once

#include <polar/component/base.h>
#include <polar/core/deltaticks.h>

namespace polar::component::clock {
	class base : public component::base {
	  private:
		DeltaTicks accumulator = 0;
	  public:
		DeltaTicks timestep;

		base(math::decimal ts = math::decimal(1) / math::decimal(50)) {
			timestep.SetSeconds(ts);
		}

		bool serialize(core::store_serializer &s) const override {
			s << timestep;
			return true;
		}

		auto frequency() const {
			return math::decimal(1) / timestep.Seconds();
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
} // namespace polar::component::clock
