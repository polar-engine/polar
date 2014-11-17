#include "common.h"
#include "Object.h"

Object::~Object() {
	for(auto component : _components) {
		delete component;
	}
}

void Object::AddComponent(Component *component) {
	_components.push_back(component);
	component->jobManager = jobManager;
	component->eventManager = eventManager;
}

void Object::Init() {
	for(auto component : _components) {
		component->Init();
	}
}

void Object::Update(int dt) {
	for(auto component : _components) {
		component->Update(dt);
	}
}

void Object::Destroy() {
	for(auto component : _components) {
		component->Destroy();
	}
}
