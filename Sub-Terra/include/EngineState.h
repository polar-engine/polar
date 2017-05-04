#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/container/vector.hpp>
#include "Destructor.h"
#include "EngineStack.h"

class EngineState {
private:
	Polar *engine;
	EntityBase<System> systems;
	boost::container::vector<boost::shared_ptr<System>> orderedSystems;
public:
	const std::string name;
	boost::container::vector<boost::shared_ptr<Destructor>> dtors;
	boost::unordered_map<std::string, Transition> transitions;

	EngineState(const std::string &name, Polar *engine) : name(name), engine(engine) {}

	~EngineState() noexcept {
		/* release destructors before systems in case of dependencies */
		dtors.clear();

		/* explicitly release shared_ptrs in unordered_map
		 * and then pop_back to destruct in reverse order
		 */
		systems.~EntityBase();
		while(!orderedSystems.empty()) {
			orderedSystems.pop_back();
		}
	}

	inline void Init() {
		for(auto &system : orderedSystems) {
			system->Init();
		}
	}

	inline void Update(DeltaTicks &dt) {
		/* copy ordered systems vector to avoid invalidation */
		auto tmpSystems = orderedSystems;
		for(auto &system : tmpSystems) {
			system->Update(dt);
		}
	}

	template<typename T, typename ...Ts> inline void AddSystem(Ts && ...args) {
		if(T::IsSupported()) {
			systems.Add<T>(engine, std::forward<Ts>(args)...);
			orderedSystems.emplace_back(systems.Get<T>());
		} else {
			std::string msg = typeid(T).name() + std::string(": unsupported");
			DebugManager()->Fatal(msg);
		}
	}

	template<typename B, typename T, typename ...Ts> inline void AddSystemAs(Ts && ...args) {
		static_assert(std::is_base_of<B, T>::value, "AddSystemAs requires base class and sub class");

#ifdef _DEBUG
		if(!T::IsSupported()) {
			std::string msg = typeid(T).name() + std::string(": unsupported");
			ENGINE_THROW(msg);
		} else {
#endif
			systems.AddAs<B, T>(engine, std::forward<Ts>(args)...);
			orderedSystems.emplace_back(systems.Get<B>());
#ifdef _DEBUG
		}
#endif
	}

	template<typename T> inline void RemoveSystem() {
		auto sys = boost::static_pointer_cast<System>(systems.Get<T>().lock());
		if(sys) {
			orderedSystems.erase(std::remove(orderedSystems.begin(), orderedSystems.end(), sys));
			systems.Remove<T>();
		}
	}

	template<typename T> inline boost::weak_ptr<T> GetSystem() {
		return systems.Get<T>();
	}

	inline void ComponentAdded(IDType id, const std::type_info *ti, boost::shared_ptr<Component> ptr) {
		for(auto &pairSystem : *systems.Get()) {
			pairSystem.second->ComponentAdded(id, ti, ptr);
		}
	}

	inline void ComponentRemoved(IDType id, const std::type_info *ti) {
		for(auto &pairSystem : *systems.Get()) {
			pairSystem.second->ComponentRemoved(id, ti);
		}
	}
};
