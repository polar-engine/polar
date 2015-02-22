#include "common.h"
#include "Polar.h"

Polar::~Polar() {
	for(auto object : _objects) {
		delete object;
	}
}

void Polar::Init() {
	if(_initDone) { ENGINE_THROW("Polar: already initialized"); }

	for(auto &system : *systems.Get()) {
		system.second->Init();
	}

	/* iterate again in case any systems added objects during initialization */
	for(auto &system : *systems.Get()) {
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
	for(auto system = ss->begin(); system != ss->end(); ++system) {
		system->second->Destroy();
	}
}

void Polar::AddObject(Object *object) {
	if(object == nullptr) { return; }

	_objects.push_back(object);
	for(auto &pair : *object->Get()) {
		_components.emplace(pair.first, object);
	}

	if(_initDone) {
		for(auto &system : *systems.Get()) {
			system.second->ObjectAdded(object);
		}
	}
}

void Polar::RemoveObject(Object *object, const bool doDelete) {
	if(object == nullptr) { return; }
	_objects.erase(std::remove(_objects.begin(), _objects.end(), object), _objects.end());
	for(auto &system : *systems.Get()) {
		system.second->ObjectRemoved(object);
	}
	if(doDelete) { delete object; }
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
