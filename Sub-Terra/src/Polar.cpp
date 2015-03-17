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
	for(auto system : orderedSystems) {
		system->Update(dt);
	}
}

void Polar::Destroy() {
	for(auto it = orderedSystems.rbegin(); it != orderedSystems.rend(); ++it) {
		(*it)->Destroy();
	}
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
