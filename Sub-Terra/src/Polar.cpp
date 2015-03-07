#include "common.h"
#include "Polar.h"

void Polar::Init() {
	if(_initDone) { ENGINE_THROW("Polar: already initialized"); }

	for(auto &system : *systems.Get()) {
		system.second->Init();
	}

	_initDone = true;
}

void Polar::Update(DeltaTicks &dt) {
	for(auto &system : *systems.Get()) {
		system.second->Update(dt);
	}
}

void Polar::Destroy() {
	auto ss = systems.Get();
	for(auto system = ss->begin(); system != ss->end(); ++system) {
		system->second->Destroy();
	}
}

void Polar::Run() {
	_running = true;

	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), then;
	//DeltaTicks iteration = DeltaTicks(ENGINE_TICKS_PER_SECOND / 200);

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
