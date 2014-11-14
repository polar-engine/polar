#pragma once

#include <string>
#include <vector>
#include "Tag.h"
#include "System.h"
#include "Component.h"
#include "Object.h"
#include "JobManager.h"

class Polar {
private:
	JobManager _jobManager;
	std::vector<System *> _systems;
	std::vector<Object *> _objects;
	template<typename T> void AddSystemImpl(const std::string &, Tag<T>);
	template<typename T, typename ...Ts> void AddSystemImpl(const std::string &, Tag<T>, Tag<Ts> ...);
	void Init();
	void Update(int);
	void Destroy();
public:
	Polar() {}
	virtual ~Polar();
	template<typename ...Ts> void AddSystem(const std::string & = "no supported systems");
	void AddObject(std::initializer_list<Component *>);
	void Run();
};

template<typename T>
void Polar::AddSystemImpl(const std::string &msg, Tag<T>) {
	if (T::IsSupported()) {
		_systems.push_back(new T());
	} else {
		throw std::runtime_error(msg);
	}
}

template<typename T, typename ...Ts>
void Polar::AddSystemImpl(const std::string &msg, Tag<T>, Tag<Ts> ...) {
	if (T::IsSupported()) {
		_systems.push_back(new T());
	} else {
		AddSystemImpl(msg, Tag<Ts>{}...);
	}
}

template<typename ...Ts>
void Polar::AddSystem(const std::string &msg) {
	AddSystemImpl(msg, Tag<Ts>{}...);
}
