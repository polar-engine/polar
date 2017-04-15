#pragma once

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

template<typename C> class EntityBase {
	typedef boost::unordered_map<const std::type_info *, boost::shared_ptr<C>> ComponentsType;
private:
	ComponentsType components;
public:
	virtual ~EntityBase() {}
	template<typename T> inline void Add() {
		if(!Has<T>()) { Add(new T()); }
	}

	template<typename T, typename ...Ts> inline void Add(Ts && ...args) {
		AddAs<T, T>(std::forward<Ts>(args)...);
	}

	template<typename B, typename T, typename ...Ts> inline void AddAs(Ts && ...args) {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::AddAs requires base class and sub class");
		Add(boost::shared_ptr<B>(new T(std::forward<Ts>(args)...)));
	}

	template<typename T> inline void Add(T *component) {
		Add(boost::shared_ptr<T>(component));
	}

	template<typename T> inline void Add(boost::shared_ptr<T> ptr) {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Add requires object of correct type");
		components.emplace(&typeid(T), boost::static_pointer_cast<C>(ptr));
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

	template<typename T> inline boost::weak_ptr<T> Get() const {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Get requires template argument of correct type");
		auto it = components.find(&typeid(T));
		if(it == components.end()) {
			return boost::weak_ptr<T>();
		} else {
			return boost::weak_ptr<T>(boost::static_pointer_cast<T, C>(it->second));
		}
	}

	inline const ComponentsType * const Get() const { return &components; }
};
