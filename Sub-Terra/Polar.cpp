#include "Polar.h"
#include <iostream>
#include <Windows.h>

Polar::~Polar() {
	for (auto system : _systems) {
		delete system;
	}
	for (auto object : _objects) {
		delete object;
	}
}

void Polar::Init() {
	_jobManager.Init();
	for (auto system : _systems) {
		system->Init();
	}
	for (auto object : _objects) {
		object->Init();
	}
}

void Polar::Update(int ts) {
	_jobManager.Update(ts);
	for (auto system : _systems) {
		system->Update(ts);
	}
}

void Polar::Destroy() {
	for (auto object : _objects) {
		object->Destroy();
	}
	for (auto system = _systems.rbegin(); system != _systems.rend(); ++system) {
		(*system)->Destroy();
	}
	_jobManager.Destroy();
}

void Polar::AddObject(std::initializer_list<Component *> components) {
	Object *object = new Object();
	object->jobManager = &_jobManager;
	for (auto component : components) {
		component->jobManager = &_jobManager;
		object->AddComponent(component);
	}
	_objects.push_back(object);
}

void Polar::Run() {
	Init();
	while (true) {
		Update(20000);
		Sleep(20);
	}
	Destroy();
}
