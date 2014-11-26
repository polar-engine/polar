#include "common.h"
#include "Polar.h"

Polar::~Polar() {
	for(auto object : _objects) {
		delete object;
	}
}

void Polar::Init() {
	for(auto &system : *systems.Get()) {
		system.second->Init();
		for(auto object : _objects) {
			system.second->ObjectAdded(object);
		}
	}
	_initDone = true;
}

void Polar::Update(DeltaTicks &dt) {
	for(auto &system : *systems.Get()) {
		system.second->Update(dt, _objects);
	}
}

void Polar::Destroy() {
	auto ss = systems.Get();
	/* unordered_map rbegin and rend don't seem to exist on some platforms */
#ifdef _WIN32
	for(auto system = ss->rbegin(); system != ss->rend(); ++system) {
#else
	for(auto system = ss->begin(); system != ss->end(); ++system) {
#endif
		system->second->Destroy();
	}
}

void Polar::AddObject(Object *object) {
	_objects.push_back(object);
	if(_initDone) {
		for(auto &system : *systems.Get()) {
			system.second->ObjectAdded(object);
		}
	}
}

void Polar::Run() {
	_running = true;
	systems.Get<EventManager>()->ListenFor("destroy", [this] (Arg) { _running = false; });
	Init();

	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), then;
	DeltaTicks iteration = DeltaTicks(ENGINE_TICKS_PER_SECOND / 200);

	while(_running) {
		then = now;
		now = std::chrono::high_resolution_clock::now();
		DeltaTicks dt = std::chrono::duration_cast<DeltaTicks>(now - then);
		Update(dt);
	}
	Destroy();
}

void Polar::Quit() {
	_running = false;
}
