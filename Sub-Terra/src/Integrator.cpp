#include "common.h"
#include "Integrator.h"
#include "IntegrableProperty.h"
#include "EventManager.h"

void Integrator::Update(DeltaTicks &dt) {
	accumulator += dt;
	auto seconds = dt.Seconds();

	while(accumulator >= timestep) {
		Tick(timestep.Seconds());
		accumulator -= timestep;
	}
}

void Integrator::Tick(DeltaTicks::seconds_type seconds) {
	for(auto it = engine->objects.left.begin(); it != engine->objects.left.end(); ++it) {
		auto property = it->info->Get<IntegrableProperty>();
		if(property != nullptr) {
			for(auto integrable : *property->GetIntegrables()) {
				integrable->Integrate(seconds);
			}
		}
	}
	engine->systems.Get<EventManager>()->FireIn("integrator", "ticked", seconds);
}
