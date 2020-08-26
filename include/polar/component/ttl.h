#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class ttl : public base {
	  public:
		DeltaTicks lifetime;
		DeltaTicks accumulator = 0;

		ttl(math::decimal seconds) {
			lifetime.SetSeconds(seconds);
		}

		void accumulate(DeltaTicks dt) {
			accumulator += dt;
		}

		bool alive() const {
			return accumulator < lifetime;
		}

		virtual std::string name() const override { return "ttl"; }
	};
} // namespace polar::component
