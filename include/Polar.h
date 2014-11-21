#pragma once

#include "System.h"
#include "Object.h"

class Polar {
private:
	bool _initDone = false;
	std::unordered_map<const std::type_info *, std::unique_ptr<System>> _systems;
	std::vector<Object *> _objects;
	void Init();
	void Update(DeltaTicks &);
	void Destroy();
public:
	Polar() {}
	virtual ~Polar();

	template<typename T> void AddSystem() {
		AddSystem(new T(this));
	}

	template<typename T, typename ...Ts> void AddSystem(Ts && ...args) {
		AddSystem(new T(this, std::forward<Ts>(args)...));
	}

	template<typename T> void AddSystem(T *system) {
		static_assert(std::is_base_of<System, T>::value, "AddSystem requires a System");
		_systems.emplace(&typeid(T), std::unique_ptr<System>(system));
	}

	template<typename T> void RemoveSystem() {
		static_assert(std::is_base_of<System, T>::value, "RemoveSystem requires a System");
		_systems.erase(&typeid(T));
	}

	template<typename T> bool HasSystem() const {
		static_assert(std::is_base_of<System, T>::value, "HasSystem requires a System");
		return HasSystem(&typeid(T));
	}

	bool HasSystem(const std::type_info *ti) const {
		return _systems.find(ti) != _systems.end();
	}

	template<typename T> T * GetSystem() const {
		static_assert(std::is_base_of<System, T>::value, "GetSystem requires a System");
		auto it = _systems.find(&typeid(T));
		if(it == _systems.end()) { return nullptr; } else { return static_cast<T *>(it->second.get()); }
	}

	void AddObject(Object *);
	void Run();
};
