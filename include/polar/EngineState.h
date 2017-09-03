#pragma once

#include <vector>
#include <unordered_map>
#include "Destructor.h"
#include "EngineStack.h"

class EngineState {
private:
	Polar *engine;
	EntityBase<System> systems;
	std::vector<std::shared_ptr<System>> orderedSystems;
public:
	const std::string name;
	std::vector<std::shared_ptr<Destructor>> dtors;
	std::unordered_map<std::string, Transition> transitions;

	EngineState(const std::string &name, Polar *engine) : name(name), engine(engine) {}

	~EngineState() noexcept {
		/* release destructors before systems in case of dependencies */
		dtors.clear();

		/* explicitly release shared_ptrs in unordered_map
		 * and then pop_back to destruct in reverse order
		 */
		systems.Clear();
		while(!orderedSystems.empty()) {
			orderedSystems.pop_back();
		}
	}

	inline void Init() {
		for(auto &system : orderedSystems) {
			DebugManager()->Debug("initing system: ", typeid(*system).name());
			system->Init();
			DebugManager()->Debug("inited system");
		}
	}

	inline void Update(DeltaTicks &dt) {
		/* copy ordered systems vector to avoid invalidation */
		auto tmpSystems = orderedSystems;
		for(auto &system : tmpSystems) {
			DebugManager()->Trace("updating system: ", typeid(*system).name());
			system->Update(dt);
			DebugManager()->Trace("updated system");
		}
	}

	template<typename T, typename ...Ts> inline void AddSystem(Ts && ...args) {
		AddSystemAs<T, T>(std::forward<Ts>(args)...);
	}

	template<typename B, typename T, typename ...Ts> inline void AddSystemAs(Ts && ...args) {
		static_assert(std::is_base_of<B, T>::value, "AddSystemAs requires base class and sub class");

#ifdef _DEBUG
		if(!T::IsSupported()) {
			DebugManager()->Fatal("unsupported system: ", typeid(T).name());
		} else {
#endif
			systems.AddAs<B, T>(engine, std::forward<Ts>(args)...);
			orderedSystems.emplace_back(systems.Get<B>());
#ifdef _DEBUG
		}
#endif
	}

	template<typename T> inline void RemoveSystem() {
		auto sys = std::static_pointer_cast<System>(systems.Get<T>().lock());
		if(sys) {
			orderedSystems.erase(std::remove(orderedSystems.begin(), orderedSystems.end(), sys));
			systems.Remove<T>();
		}
	}

	template<typename T> inline std::weak_ptr<T> GetSystem() {
		return systems.Get<T>();
	}

	inline void ComponentAdded(IDType id, const std::type_info *ti, std::shared_ptr<Component> ptr) {
		for(auto &pairSystem : *systems.Get()) {
			auto &system = pairSystem.second;
			DebugManager()->Trace("notifying system of component added: ", typeid(*system).name(), ", ", ti->name());
			system->ComponentAdded(id, ti, ptr);
			DebugManager()->Trace("notified system of component added");
		}
	}

	inline void ComponentRemoved(IDType id, const std::type_info *ti) {
		for(auto &pairSystem : *systems.Get()) {
			auto &system = pairSystem.second;
			DebugManager()->Trace("notifying system of component removed: ", typeid(*system).name(), ", ", ti->name());
			system->ComponentRemoved(id, ti);
			DebugManager()->Trace("notified system of component removed");
		}
	}
};
