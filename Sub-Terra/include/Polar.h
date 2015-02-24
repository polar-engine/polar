#pragma once

#include <map>
#include <boost/range/iterator_range.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include "System.h"

class Polar {
public:
	typedef boost::bimap<
		//boost::bimaps::unordered_multiset_of<std::uint_fast64_t>,
		boost::bimaps::multiset_of<Object *>,
		boost::bimaps::unordered_multiset_of<const std::type_info *>,
		boost::bimaps::with_info<Component *>
	> ComponentsBimap;
	ComponentsBimap components;
private:
	bool _initDone = false;
	bool _running = false;

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

	template<typename T> auto GetObjectsWithComponent() -> decltype(components.right.equal_range(&typeid(T))) {
		static_assert(std::is_base_of<Component, T>::value, "Polar::GetObjectsWithComponent requires object of type Component");
		return components.right.equal_range(&typeid(T));
	}

	void AddObject(Object *);
	void RemoveObject(Object *, const bool doDelete = true);
	void Init();
	void Run();
	void Destroy();

	void Quit();
};
