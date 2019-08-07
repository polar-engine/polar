#pragma once

#include <atomic>
#include <cstdint>
#include <polar/support/integrator/integrable.h>
#include <polar/system/base.h>

namespace polar::system {
	class integrator : public base {
	  private:
		DeltaTicks accumulator;
		void tick(DeltaTicks::seconds_type);

	  protected:
		void update(DeltaTicks &) override;

	  public:
		const int fps             = 50;
		const DeltaTicks timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / fps);
		std::atomic_uint_fast32_t deltaMicroseconds = {0};

		static bool supported() { return true; }
		integrator(core::polar *engine) : base(engine) {}

		const inline DeltaTicks &getaccumulator() const { return accumulator; }

		inline void force_tick() { tick(timestep.Seconds()); }

		void revert_by(size_t = 0);
		void revert_to(size_t = 0);
	};
} // namespace polar::system
