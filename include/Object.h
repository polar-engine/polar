#pragma once

#include "Component.h"

class Object {
private:
	std::unordered_map<const std::type_info *, std::unique_ptr<Component>> _components;
public:
	Object() {}
	virtual ~Object() {}

	template<typename T> void AddComponent() {
		AddComponent(new T());
	}

	template<typename T, typename ...Ts> void AddComponent(Ts && ...args) {
		AddComponent(new T(std::forward<Ts>(args)...));
	}

	template<typename T> void AddComponent(T *component) {
		static_assert(std::is_base_of<Component, T>::value, "AddComponent requires a Component");
		_components.emplace(&typeid(T), std::unique_ptr<Component>(component));
	}

	template<typename T> void RemoveComponent() {
		static_assert(std::is_base_of<Component, T>::value, "RemoveComponent requires a Component");
		_components.erase(&typeid(T));
	}

	template<typename T> bool HasComponent() {
		static_assert(std::is_base_of<Component, T>::value, "HasComponent requires a Component");
		return HasComponent(&typeid(T));
	}

	bool HasComponent(const std::type_info *ti) {
		return _components.find(ti) != _components.end();
	}

	template<typename T> T * GetComponent() {
		static_assert(std::is_base_of<Component, T>::value, "GetComponent requires a Component");
		auto it = _components.find(&typeid(T));
		if(it == _components.end()) { return nullptr; } else { return static_cast<T *>(it->second.get()); }
	}
};
