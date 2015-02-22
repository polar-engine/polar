#pragma once

#include <unordered_map>
#include "System.h"

class Polar {
public:
	typedef std::unordered_multimap<const std::type_info *, Object *> ComponentsType;
private:
	bool _initDone = false;
	bool _running = false;
	std::vector<EntityBase<Component> *> _objects;
	ComponentsType _components;

	void Update(DeltaTicks &);
public:
	EntityBase<System> systems;

	Polar() {}
	virtual ~Polar();

	template<typename T> void AddSystem() {
		if(T::IsSupported()) {
			systems.Add<T>(this);
		} else {
			std::string msg = typeid(T).name() + std::string(": unsupported");
			ENGINE_THROW(msg);
		}
	}

	template<typename T, typename ...Ts> void AddSystem(Ts && ...args) {
		if(T::IsSupported()) {
			systems.Add<T>(this, std::forward<Ts>(args)...);
		} else {
			std::string msg = typeid(T).name() + std::string(": unsupported");
			ENGINE_THROW(msg);
		}
	}

	template<typename T> std::pair<Polar::ComponentsType::const_iterator, Polar::ComponentsType::const_iterator> GetObjectsWithComponent() {
		static_assert(std::is_base_of<Component, T>::value, "Polar::GetObjectsWithComponent requires object of type Component");
		return _components.equal_range(&typeid(T));
	}

	void AddObject(Object *);
	void RemoveObject(Object *, const bool doDelete = true);
	void Init();
	void Run();
	void Destroy();

	void Quit();
};
