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
	}
}

void Polar::Update(DeltaTicks dt) {
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

Object * Polar::AddObject() {
	Object *object = new Object();
	_objects.push_back(object);
	return object;
}

void Polar::Run() {
	Init();
	bool running = true;
	_eventManager.ListenFor("destroy", [&running] (Arg) { running = false; });
	while(running) {
		Update(DeltaTicks(2));
		std::this_thread::sleep_for(DeltaTicks(2));
	}
	Destroy();
}
