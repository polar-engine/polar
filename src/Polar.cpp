#include "common.h"
#include "Polar.h"

Polar::~Polar() {
	for(auto system : _systems) {
		delete system;
	}
	for(auto object : _objects) {
		delete object;
	}
}

void Polar::Init() {
	_jobManager.Init();
	_eventManager.Init();
	for(auto system : _systems) {
		system->Init();
		for(auto object : _objects) {
			system->ObjectAdded(object);
		}
	}
	_initDone = true;
}

void Polar::Update(DeltaTicks &dt) {
	_jobManager.Update(dt, _objects);
	_eventManager.Update(dt, _objects);
	for(auto system : _systems) {
		system->Update(dt, _objects);
	}
}

void Polar::Destroy() {
	for(auto system = _systems.rbegin(); system != _systems.rend(); ++system) {
		(*system)->Destroy();
	}
	_eventManager.Destroy();
	_jobManager.Destroy();
}

void Polar::AddSystem(System *system) {
	system->jobManager = &_jobManager;
	system->eventManager = &_eventManager;
	_systems.push_back(system);
}

void Polar::AddObject(Object *object) {
	_objects.push_back(object);
	if(_initDone) {
		for(auto system : _systems) {
			system->ObjectAdded(object);
		}
	}
}

void Polar::Run() {
	bool running = true;
	_eventManager.ListenFor("destroy", [&running] (Arg) { running = false; });
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
