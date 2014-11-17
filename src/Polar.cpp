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
	for(auto object : _objects) {
		object->Init();
	}
}

void Polar::Update(int ts) {
	_jobManager.Update(ts);
	_eventManager.Update(ts);
	for(auto system : _systems) {
		system->Update(ts);
	}
}

void Polar::Destroy() {
	for(auto object : _objects) {
		object->Destroy();
	}
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

void Polar::AddObject(std::initializer_list<Component *> components) {
	Object *object = new Object();
	object->jobManager = &_jobManager;
	object->eventManager = &_eventManager;
	for(auto component : components) {
		object->AddComponent(component);
	}
	_objects.push_back(object);
}

void Polar::Run() {
	Init();
	bool running = true;
	_eventManager.ListenFor("destroy", [&running] (Arg) { running = false; });
	while(running) {
		Update(20000);
		std::this_thread::sleep_for(std::chrono::milliseconds(17));
	}
	Destroy();
}
