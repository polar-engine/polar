#pragma once

#include "Tag.h"
#include "System.h"
#include "Component.h"
#include "Object.h"
#include "JobManager.h"
#include "EventManager.h"

class Polar {
private:
	JobManager _jobManager;
	EventManager _eventManager;
	std::vector<System *> _systems;
	std::vector<Object *> _objects;
	template<typename T> void AddSystemImpl(const std::string &, Tag<T>);
	template<typename T, typename ...Ts> void AddSystemImpl(const std::string &, Tag<T>, Tag<Ts> ...);
	void Init();
	void Update(DeltaTicks &);
	void Destroy();
public:
	Polar() {}
	virtual ~Polar();
	template<typename ...Ts> void AddSystem(const std::string & = "no supported systems");
	void AddSystem(System *);
	Object * AddObject();
	void Run();
};

template<typename T>
void Polar::AddSystemImpl(const std::string &msg, Tag<T>) {
	if(T::IsSupported()) {
		AddSystem(new T());
	} else {
		ENGINE_ERROR(msg);
		ENGINE_CONTINUE;
		throw std::runtime_error(msg);
	}
}

template<typename T, typename ...Ts>
void Polar::AddSystemImpl(const std::string &msg, Tag<T>, Tag<Ts> ...) {
	if(T::IsSupported()) {
		AddSystem(new T());
	} else {
		AddSystemImpl(msg, Tag<Ts>()...);
	}
}

template<typename ...Ts>
void Polar::AddSystem(const std::string &msg) {
	AddSystemImpl(msg, Tag<Ts>()...);
}
