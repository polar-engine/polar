#pragma once

#include "System.h"
#include "Integrable.h"

class Integrator : public System {
private:
	DeltaTicks accumulator;
	void Tick(DeltaTicks::seconds_type, std::vector<Object *> &);
protected:
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
public:
	const DeltaTicks timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / 50);

	static bool IsSupported() { return true; }
	Integrator(Polar *engine) : System(engine) {}
	const DeltaTicks & Accumulator() { return accumulator; }
};
