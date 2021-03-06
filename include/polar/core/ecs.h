#pragma once

#include <boost/container/flat_map.hpp>
#include <memory>
#include <typeindex>

namespace polar::core {
	template<typename C> class ecs {
		using component_map_t = boost::container::flat_map<std::type_index, std::shared_ptr<C>>;

	  private:
		component_map_t components;

	  public:
		virtual ~ecs() {}

		inline void clear() { components.clear(); }

		template<typename T> inline auto add() {
			if(!has<T>()) { return add(new T()); }
		}

		// TODO: return the newly added component
		template<typename T, typename... Ts> inline auto add(Ts &&... args) {
			return add_as<T, T>(std::forward<Ts>(args)...);
		}

		template<typename B, typename T, typename... Ts> inline auto add_as(Ts &&... args) {
			static_assert(std::is_base_of<C, T>::value, "ecs::add_as requires base class and sub class");
			return add(std::shared_ptr<B>(new T(std::forward<Ts>(args)...)));
		}

		template<typename T> inline void add(T *component) { add(std::shared_ptr<T>(component)); }

		template<typename T> inline auto add(std::shared_ptr<T> ptr) {
			static_assert(std::is_base_of<C, T>::value, "ecs::add requires object of correct type");
			components.emplace(typeid(T), std::static_pointer_cast<C>(ptr));
			return ptr;
		}

		template<typename T> inline void remove() {
			static_assert(std::is_base_of<C, T>::value, "ecs::remove requires object of correct type");
			components.erase(typeid(T));
		}

		template<typename T> inline bool has() const {
			static_assert(std::is_base_of<C, T>::value, "ecs::has requires object of correct type");
			return has(typeid(T));
		}

		inline bool has(std::type_index ti) const { return components.find(ti) != components.end(); }

		inline std::shared_ptr<C> get(std::type_index ti) const {
			auto it = components.find(ti);
			if(it == components.end()) {
				return std::shared_ptr<C>();
			} else {
				return it->second;
			}
		}

		template<typename T> inline std::shared_ptr<T> get() const {
			static_assert(std::is_base_of<C, T>::value, "ecs::get requires template argument of correct type");
			auto baseptr = get(typeid(T));
			return std::static_pointer_cast<T, C>(baseptr);
		}

		inline const component_map_t *const get() const { return &components; }
	};
} // namespace polar::core
