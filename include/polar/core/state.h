#if !defined(POLAR_H)
#include <polar/core/polar.h>
#else
#pragma once

#include <polar/core/polar.h>
#include <polar/core/ref.h>
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
		std::vector<std::shared_ptr<system::base>> toErase;
		std::vector<ref> dtors;

		void insert(std::type_index, std::shared_ptr<system::base>);
	  public:
		const std::string name;
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

		inline void keep(ref r) {
			dtors.emplace_back(r);
		}

		void init();
		void update(DeltaTicks &dt);

		template<
			typename T,
			typename... Ts,
			typename = typename std::enable_if<std::is_base_of<system::base, T>::value>::type
		>
		inline auto add(Ts &&... args) {
			return add_as<T, T>(std::forward<Ts>(args)...);
		}

		template<
			typename B,
			typename T,
			typename... Ts,
			typename = typename std::enable_if<std::is_base_of<system::base, T>::value>::type,
			typename = typename std::enable_if<std::is_base_of<B, T>::value>::type
		>
		inline auto add_as(Ts &&... args) {
#ifdef _DEBUG
			if(!T::supported()) {
				debugmanager()->fatal("unsupported system: ", typeid(T).name());
				return std::shared_ptr<B>();
			} else {
#endif
				auto ptr = systems.add_as<B, T>(engine, std::forward<Ts>(args)...);
				orderedSystems.emplace_back(systems.get<B>());
				insert(typeid(B), ptr);
				return ptr;
#ifdef _DEBUG
			}
#endif
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<system::base, T>::value>::type>
		inline void remove() {
			auto sys = std::static_pointer_cast<system::base>(systems.get<T>().lock());
			if(sys) {
				systems.remove<T>();
				toErase.emplace_back(sys);
			}
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<system::base, T>::value>::type>
		inline void remove_now() {
			auto sys = std::static_pointer_cast<system::base>(systems.get<T>().lock());
			if(sys) {
				orderedSystems.erase(std::remove(orderedSystems.begin(),
				                                 orderedSystems.end(), sys));
				systems.remove<T>();
			}
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<system::base, T>::value>::type>
		inline std::weak_ptr<T> get() {
			return systems.get<T>();
		}

		inline std::weak_ptr<system::base> get(std::type_index ti) {
			return systems.get(ti);
		}

		void system_added(std::type_index, std::shared_ptr<system::base>);
		void component_added(weak_ref, std::type_index, std::shared_ptr<component::base>);
		void component_removed(weak_ref, std::type_index);
	};
} // namespace polar::core

#endif
