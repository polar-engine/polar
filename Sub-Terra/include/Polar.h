#pragma once

#include <boost/weak_ptr.hpp>
#include <boost/container/vector.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include "Component.h"
#include "System.h"

class Polar {
public:
	typedef boost::bimap<
		boost::bimaps::multiset_of<IDType>,
		boost::bimaps::unordered_multiset_of<const std::type_info *>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<std::shared_ptr<Component>>
	> Bimap;
private:
	bool _initDone = false;
	bool _running = false;

	void Update(DeltaTicks &);
public:
	EntityBase<System> systems;
	boost::container::vector<boost::shared_ptr<System>> orderedSystems;
	Bimap objects;
	IDType nextID = 1;

	Polar() {}
	~Polar();

	template<typename T, typename ...Ts> inline void AddSystem(Ts && ...args) {
		if(T::IsSupported()) {
			systems.Add<T>(this, std::forward<Ts>(args)...);
			orderedSystems.emplace_back(systems.Get<T>());
		} else {
			std::string msg = typeid(T).name() + std::string(": unsupported");
			ENGINE_THROW(msg);
		}
	}

	template<typename T> inline void RemoveSystem() {
		auto sys = boost::static_pointer_cast<System>(systems.Get<T>().lock());
		if(sys) {
			orderedSystems.erase(std::remove(orderedSystems.begin(), orderedSystems.end(), sys));
			systems.Remove<T>();
		}
	}

	inline IDType AddObject() { return nextID++; }

	inline void RemoveObject(IDType id) {
		auto pairLeft = objects.left.equal_range(id);
		for(auto it = pairLeft.first; it != pairLeft.second; ++it) {
			for(auto &pairSystem : *systems.Get()) {
				pairSystem.second->ComponentRemoved(id, it->get_right());
			}
		}
		objects.left.erase(id);
	}

	template<typename T, typename ...Ts> inline void AddComponent(IDType id, Ts && ...args) {
		InsertComponent(id, new T(std::forward<Ts>(args)...));
	}

	template<typename T> inline void InsertComponent(IDType id, T *component) {
		std::shared_ptr<Component> ptr(component);
		/*auto pair = */objects.insert(Bimap::value_type(id, &typeid(T), ptr));
		for(auto &pairSystem : *systems.Get()) {
			pairSystem.second->ComponentAdded(id, &typeid(T), ptr);
		}
	}

	template<typename T> inline T * GetComponent(IDType id) {
		auto it = objects.find(Bimap::relation(id, &typeid(T)));
		if(it != objects.end()) {
			return static_cast<T *>(it->info.get());
		} else { return nullptr; }
	}

	void Init();
	void Run();

	void Quit();
};
