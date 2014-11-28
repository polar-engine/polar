#pragma once

template<typename C> class EntityBase {
	typedef std::unordered_map<const std::type_info *, std::unique_ptr<C>> ComponentsType;
private:
	ComponentsType components;
public:
	virtual ~EntityBase() {}
	template<typename T> void Add() {
		Add(new T());
	}

	template<typename T, typename ...Ts> void Add(Ts && ...args) {
		Add(new T(std::forward<Ts>(args)...));
	}

	template<typename T> void Add(T *component) {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Add requires object of correct type");
		components.emplace(&typeid(T), std::unique_ptr<C>(component));
	}

	template<typename T> void Remove() {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Remove requires object of correct type");
		components.erase(&typeid(T));
	}

	template<typename T> bool Has() const {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Has requires object of correct type");
		return Has(&typeid(T));
	}

	bool Has(const std::type_info *ti) const {
		return components.find(ti) != components.end();
	}

	template<typename T> T * Get() const {
		static_assert(std::is_base_of<C, T>::value, "EntityBase::Get requires object of correct type");
		auto it = components.find(&typeid(T));
		if(it == components.end()) { return nullptr; } else { return static_cast<T *>(it->second.get()); }
	}

	const ComponentsType * const Get() const { return &components; }
};
