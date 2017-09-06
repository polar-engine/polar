#pragma once

#include <vector>
#include <unordered_map>
#include <polar/core/destructor.h>
#include <polar/core/stack.h>

namespace polar { namespace core {
	class state {
	private:
		polar *engine;
		ecs<system::base> systems;
		std::vector<std::shared_ptr<system::base>> orderedSystems;
	public:
		const std::string name;
		std::vector<std::shared_ptr<destructor>> dtors;
		std::unordered_map<std::string, Transition> transitions;

		state(const std::string &name, polar *engine) : name(name), engine(engine) {}

		~state() noexcept {
			/* release destructors before systems in case of dependencies */
			dtors.clear();

			/* explicitly release shared_ptrs in unordered_map
			 * and then pop_back to destruct in reverse order
			 */
			systems.clear();
			while(!orderedSystems.empty()) {
				orderedSystems.pop_back();
			}
		}

		inline void init() {
			for(auto &system : orderedSystems) {
				debugmanager()->debug("initing system: ", typeid(*system).name());
				system->init();
				debugmanager()->debug("inited system");
			}
		}

		inline void update(DeltaTicks &dt) {
			/* copy ordered systems vector to avoid invalidation */
			auto tmpSystems = orderedSystems;
			for(auto &system : tmpSystems) {
				debugmanager()->trace("updating system: ", typeid(*system).name());
				system->update(dt);
				debugmanager()->trace("updated system");
			}
		}

		template<typename T, typename ...Ts> inline void addsystem(Ts && ...args) {
			addsystem_as<T, T>(std::forward<Ts>(args)...);
		}

		template<typename B, typename T, typename ...Ts> inline void addsystem_as(Ts && ...args) {
			static_assert(std::is_base_of<B, T>::value, "addsystem_as requires base class and sub class");

	#ifdef _DEBUG
			if(!T::supported()) {
				debugmanager()->fatal("unsupported system: ", typeid(T).name());
			} else {
	#endif
				systems.add_as<B, T>(engine, std::forward<Ts>(args)...);
				orderedSystems.emplace_back(systems.get<B>());
	#ifdef _DEBUG
			}
	#endif
		}

		template<typename T> inline void removesystem() {
			auto sys = std::static_pointer_cast<system::base>(systems.get<T>().lock());
			if(sys) {
				orderedSystems.erase(std::remove(orderedSystems.begin(), orderedSystems.end(), sys));
				systems.remove<T>();
			}
		}

		template<typename T> inline std::weak_ptr<T> getsystem() {
			return systems.get<T>();
		}

		inline void componentadded(IDType id, const std::type_info *ti, std::shared_ptr<component::base> ptr) {
			for(auto &pairSystem : *systems.get()) {
				auto &system = pairSystem.second;
				debugmanager()->trace("notifying system of component added: ", typeid(*system).name(), ", ", ti->name());
				system->componentadded(id, ti, ptr);
				debugmanager()->trace("notified system of component added");
			}
		}

		inline void componentremoved(IDType id, const std::type_info *ti) {
			for(auto &pairSystem : *systems.get()) {
				auto &system = pairSystem.second;
				debugmanager()->trace("notifying system of component removed: ", typeid(*system).name(), ", ", ti->name());
				system->componentremoved(id, ti);
				debugmanager()->trace("notified system of component removed");
			}
		}
	};
} }
