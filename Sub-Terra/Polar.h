#pragma once

#include <string>
#include <vector>
#include "Tag.h"
#include "System.h"
#include "Component.h"
#include "Object.h"

class Polar {
private:
	std::vector<System *> _systems;
	std::vector<Object *> _objects;
	template<typename T> void AddSystemImpl(std::string const &, Tag<T>);
	template<typename T, typename ...Ts> void AddSystemImpl(std::string const &, Tag<T>, Tag<Ts> ...);
	void Init();
	void Destroy();
public:
	Polar() {}
	virtual ~Polar();
	template<typename ...Ts> void AddSystem(std::string const & = "no supported systems");
	void AddObject(std::initializer_list<Component *>);
	void Run();
};

template<typename T>
void Polar::AddSystemImpl(std::string const &msg, Tag<T>) {
	if (T::IsSupported()) {
		_systems.push_back(new T());
	} else {
		throw std::runtime_error(msg);
	}
}

template<typename T, typename ...Ts>
void Polar::AddSystemImpl(std::string const &msg, Tag<T>, Tag<Ts> ...) {
	if (T::IsSupported()) {
		_systems.push_back(new T());
	} else {
		AddSystemImpl(msg, Tag<Ts>{}...);
	}
}

template<typename ...Ts>
void Polar::AddSystem(std::string const &msg) {
	AddSystemImpl(msg, Tag<Ts>{}...);
}
