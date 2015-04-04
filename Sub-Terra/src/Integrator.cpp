#include "common.h"
#include "Integrator.h"
#include "IntegrableProperty.h"
#include "EventManager.h"

void Integrator::Update(DeltaTicks &dt) {
	accumulator += dt;
	if(accumulator.Seconds() > 1.0f) { accumulator.SetSeconds(1.0f); }

	while(accumulator >= timestep) {
		Tick(timestep.Seconds());
		accumulator -= timestep;
	}
}

void Integrator::Tick(DeltaTicks::seconds_type seconds) {
	for(auto it = engine->objects.left.begin(); it != engine->objects.left.end(); ++it) {
		auto property = it->info->Get<IntegrableProperty>().lock();
		if(property) {
			for(auto integrable : *property->GetIntegrables()) {
				integrable->Integrate(seconds);
			}
		}
	}
	engine->systems.Get<EventManager>().lock()->FireIn("integrator", "ticked", seconds);
}
