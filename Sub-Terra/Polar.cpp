#include "Polar.h"
#include <utility>

Polar::~Polar() {
	for (auto system : _systems) {
		delete system;
	}
	for (auto object : _objects) {
		delete object;
	}
}

void Polar::Init() {
	for (auto system : _systems) {
		system->Init();
	}
	for (auto object : _objects) {
		object->Init();
	}
}

void Polar::Destroy() {
	for (auto system : _systems) {
		system->Destroy();
	}
	for (auto object : _objects) {
		object->Destroy();
	}
}

void Polar::AddObject(std::initializer_list<Component *> components) {
	Object *object = new Object();
	for (auto component : components) {
		object->AddComponent(component);
	}
	_objects.push_back(object);
}

void Polar::Run() {
	Init();
	Destroy();
}
