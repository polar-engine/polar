#if !defined(POLAR_H)
#include <polar/core/polar.h>
#else
#pragma once

#include <polar/core/destructor.h>
#include <polar/core/polar.h>
#include <polar/core/stack.h>
#include <polar/system/base.h>
#include <unordered_map>
#include <vector>

namespace polar::core {
	class state {
	  private:
		polar *engine;
		ecs<system::base> systems;
		std::vector<std::shared_ptr<system::base>> orderedSystems;

	  public:
		const std::string name;
		std::vector<std::shared_ptr<destructor>> dtors;
		std::unordered_map<std::string, Transition> transitions;

		state(const std::string &name, polar *engine)
		    : engine(engine), name(name) {}

		~state() noexcept {
			/* release destructors before systems in case of dependencies */
			dtors.clear();

			/* explicitly release shared_ptrs in unordered_map
			 * and then pop_back to destruct in reverse order
			 */
			systems.clear();
			while(!orderedSystems.empty()) { orderedSystems.pop_back(); }
		}

		void init();
		void update(DeltaTicks &dt);

		template<typename T, typename... Ts,
		         typename = typename std::enable_if<
		             std::is_base_of<system::base, T>::value>::type>
		inline void add(Ts &&... args) {
			add_as<T, T>(std::forward<Ts>(args)...);
		}

		template<typename B, typename T, typename... Ts,
		         typename = typename std::enable_if<
		             std::is_base_of<system::base, T>::value>::type,
		         typename = typename std::enable_if<
		             std::is_base_of<B, T>::value>::type>
		inline void add_as(Ts &&... args) {
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

		template<typename T, typename = typename std::enable_if<
		                         std::is_base_of<system::base, T>::value>::type>
		inline void remove() {
			auto sys =
			    std::static_pointer_cast<system::base>(systems.get<T>().lock());
			if(sys) {
				orderedSystems.erase(std::remove(orderedSystems.begin(),
				                                 orderedSystems.end(), sys));
				systems.remove<T>();
			}
		}

		template<typename T, typename = typename std::enable_if<
		                         std::is_base_of<system::base, T>::value>::type>
		inline std::weak_ptr<T> get() {
			return systems.get<T>();
		}

		inline std::weak_ptr<system::base> get(std::type_index ti) {
			return systems.get(ti);
		}

		void component_added(IDType id, std::type_index ti,
		                     std::shared_ptr<component::base> ptr);
		void component_removed(IDType id, std::type_index ti);
	};
} // namespace polar::core

#endif
