#include "common.h"
#include "Polar.h"

Polar::~Polar() {
	for(auto object : _objects) {
		delete object;
	}
}

void Polar::Init() {
	for(auto &system : _systems) {
		system.second->Init();
		for(auto object : _objects) {
			system.second->ObjectAdded(object);
		}
	}
	_initDone = true;
}

void Polar::Update(DeltaTicks &dt) {
	for(auto &system : _systems) {
		system.second->Update(dt, _objects);
	}
}

void Polar::Destroy() {
	for(auto system = _systems.rbegin(); system != _systems.rend(); ++system) {
		system->second->Destroy();
	}
}

void Polar::AddObject(Object *object) {
	_objects.push_back(object);
	if(_initDone) {
		for(auto &system : _systems) {
			system.second->ObjectAdded(object);
		}
	}
}

void Polar::Run() {
	bool running = true;
	GetSystem<EventManager>()->ListenFor("destroy", [&running] (Arg) { running = false; });
	Init();

	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), then;
	DeltaTicks iteration = DeltaTicks(ENGINE_TICKS_PER_SECOND / 200);

	while(running) {
		then = now;
		now = std::chrono::high_resolution_clock::now();
		DeltaTicks dt = std::chrono::duration_cast<DeltaTicks>(now - then);
		Update(dt);
	}
	Destroy();
}
