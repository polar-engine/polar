#include "common.h"
#include "Integrator.h"
#include "IntegrableProperty.h"

void Integrator::Update(DeltaTicks &dt, std::vector<Object *> &objects) {
	accumulator += dt;
	auto seconds = dt.Seconds();

	while(accumulator >= timestep) {
		Tick(timestep.Seconds(), objects);
		accumulator -= timestep;
	}
}

void Integrator::Tick(DeltaTicks::seconds_type seconds, std::vector<Object *> &objects) {
	for(auto object : objects) {
		for(auto &pair : *object->Get()) {
			auto property = pair.second->Get<IntegrableProperty>();
			if(property != nullptr) {
				for(auto integrable : *property->GetIntegrables()) {
					integrable->Integrate(seconds);
				}
			}
		}
	}
}
