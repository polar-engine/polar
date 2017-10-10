#pragma once

#include <atomic>
#include <cstdint>
#include <polar/support/integrator/integrable.h>
#include <polar/system/base.h>

namespace polar {
namespace system {
	class integrator : public base {
	  private:
		DeltaTicks accumulator;
		void tick(DeltaTicks::seconds_type);

	  protected:
		void update(DeltaTicks &) override final;

	  public:
		const int fps             = 50;
		const DeltaTicks timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / fps);
		std::atomic_uint_fast32_t alphaMicroseconds = {0};

		static bool supported() { return true; }
		integrator(core::polar *engine) : base(engine) {}
		const inline DeltaTicks &getaccumulator() const { return accumulator; }
	};
}
}
