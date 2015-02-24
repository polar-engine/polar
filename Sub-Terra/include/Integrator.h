#pragma once

#include "System.h"
#include "Integrable.h"

class Integrator : public System {
private:
	DeltaTicks accumulator;
	void Tick(DeltaTicks::seconds_type);
protected:
	void Update(DeltaTicks &) override final;
public:
	const int fps = 50;
	const DeltaTicks timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / fps);

	static bool IsSupported() { return true; }
	Integrator(Polar *engine) : System(engine) {}
	const DeltaTicks & Accumulator() { return accumulator; }
};
