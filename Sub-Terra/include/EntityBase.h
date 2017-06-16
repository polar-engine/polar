#pragma once

#include <unordered_map>

template<typename C> class EntityBase {
	typedef std::unordered_map<const std::type_info *, std::shared_ptr<C>> ComponentsType;
private:
	ComponentsType components;
public:
	inline void Clear() {
		components.clear();
	}

	template<typename T> inline void Add() {
		if(!Has<T>()) { Add(new T()); }
	}

	template<typename T, typename ...Ts> inline void Add(Ts && ...args) {
		AddAs<T, T>(std::forward<Ts>(args)...);
	}

	template<typename B, typename T, typename ...Ts> inline void AddAs(Ts && ...args) {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::AddAs requires base class and sub class");
		Add(std::shared_ptr<B>(new T(std::forward<Ts>(args)...)));
	}

	template<typename T> inline void Add(T *component) {
		Add(std::shared_ptr<T>(component));
	}

	template<typename T> inline void Add(std::shared_ptr<T> ptr) {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Add requires object of correct type");
		components.emplace(&typeid(T), std::static_pointer_cast<C>(ptr));
	}

	template<typename T> inline void Remove() {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Remove requires object of correct type");
		components.erase(&typeid(T));
	}

	template<typename T> inline bool Has() const {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Has requires object of correct type");
		return Has(&typeid(T));
	}

	inline bool Has(const std::type_info *ti) const {
		return components.find(ti) != components.end();
	}

	template<typename T> inline std::weak_ptr<T> Get() const {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Get requires template argument of correct type");
		auto it = components.find(&typeid(T));
		if(it == components.end()) {
			return std::weak_ptr<T>();
		} else {
			return std::weak_ptr<T>(std::static_pointer_cast<T, C>(it->second));
		}
	}

	inline const ComponentsType * const Get() const { return &components; }
};
