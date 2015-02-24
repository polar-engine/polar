#include "common.h"
#include "Integrator.h"
#include "IntegrableProperty.h"

void Integrator::Update(DeltaTicks &dt) {
	accumulator += dt;
	auto seconds = dt.Seconds();

	while(accumulator >= timestep) {
		Tick(timestep.Seconds());
		accumulator -= timestep;
	}
}

void Integrator::Tick(DeltaTicks::seconds_type seconds) {
	for(auto it = engine->components.left.begin(); it != engine->components.left.end(); ++it) {
		auto property = it->info->Get<IntegrableProperty>();
		if(property != nullptr) {
			for(auto integrable : *property->GetIntegrables()) {
				integrable->Integrate(seconds);
			}
		}
	}
}
