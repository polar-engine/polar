#include "common.h"
#include "Polar.h"

Polar::~Polar() {
	for(auto it = components.left.begin(); it != components.left.end(); it = components.left.upper_bound(it->first)) {
		delete it->first;
	}
}

void Polar::Init() {
	if(_initDone) { ENGINE_THROW("Polar: already initialized"); }

	for(auto &system : *systems.Get()) {
		system.second->Init();
	}

	/* iterate again in case any systems added objects during initialization */
	for(auto &system : *systems.Get()) {
		for(auto it = components.left.begin(); it != components.left.end(); ++it) {
			system.second->ObjectAdded(it->first);
		}
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

void Polar::AddObject(Object *object) {
	if(object == nullptr) { return; }

	for(auto &pair : *object->Get()) {
		components.insert(ComponentsBimap::value_type(object, pair.first, pair.second.get()));
	}

	if(_initDone) {
		for(auto &system : *systems.Get()) {
			system.second->ObjectAdded(object);
		}
	}
}

void Polar::RemoveObject(Object *object, const bool doDelete) {
	if(object == nullptr) { return; }

	components.left.erase(object);
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
