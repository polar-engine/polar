#include "common.h"
#include "Polar.h"

void Polar::Init() {
	if(_initDone) { ENGINE_THROW("Polar: already initialized"); }

	for(auto &system : orderedSystems) {
		system->Init();
	}

	_initDone = true;
}

void Polar::Update(DeltaTicks &dt) {
	/* copy ordered systems vector to avoid invalidation */
	auto tmpSystems = orderedSystems;
	for(auto &system : tmpSystems) {
		system->Update(dt);
	}
}

Polar::~Polar() {
	/* explicitly release shared_ptrs in unordered_map
	 * and then pop_back to destruct in reverse order
	 */
	systems.~EntityBase();
	while(!orderedSystems.empty()) { orderedSystems.pop_back(); }
}

void Polar::Run() {
	_running = true;

	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), then;

	while(_running) {
		then = now;
		now = std::chrono::high_resolution_clock::now();
		DeltaTicks dt = std::chrono::duration_cast<DeltaTicksBase>(now - then);
		Update(dt);
	}
}

void Polar::Quit() {
	_running = false;
}
