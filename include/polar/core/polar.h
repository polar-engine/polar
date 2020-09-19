#pragma once

#ifndef POLAR_H
#define POLAR_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <map>
#include <polar/core/log.h>
#include <polar/core/stack.h>
#include <polar/math/types.h>
#include <polar/util/buildinfo.h>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// define types in explicit order to stop loops

namespace polar::core {
	class polar;
}

#include <polar/core/store.h>

#include <polar/component/base.h>
#include <polar/core/mutable_component.h>

#include <polar/system/base.h>

#include <polar/core/state.h>

namespace polar::core {
	namespace index {
		struct ref           {};
		struct ti            {};
		struct rel_ordered   {};
		struct rel_unordered {};
	} // namespace index

	struct relation {
		struct pair_comp {
			inline bool operator()(const relation &lhs, const relation &rhs) const {
				if(lhs.r != rhs.r) {
					return lhs.r < rhs.r;
				} else {
					return lhs.ti < rhs.ti;
				}
			}
		};

		weak_ref r;
		std::type_index ti;
		std::shared_ptr<component::base> ptr;

		friend inline size_t hash_value(const relation &rel) {
			size_t seed = 0;
			boost::hash_combine(seed, rel.r);
			boost::hash_combine(seed, rel.ti);
			return seed;
		}

		friend inline bool operator==(const relation &lhs, const relation &rhs) {
			return lhs.r == rhs.r && lhs.ti == rhs.ti;
		}
	};

	class polar {
	  public:
		using priority_t        = support::debug::priority;
		using state_initializer = std::function<void(polar *, state &)>;

		using bimap = boost::multi_index_container<
			relation,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_non_unique<boost::multi_index::tag<index::ref          >, boost::multi_index::member<relation, weak_ref,        &relation::r>>,
				boost::multi_index::ordered_non_unique<boost::multi_index::tag<index::ti           >, boost::multi_index::member<relation, std::type_index, &relation::ti>>,
				boost::multi_index::ordered_unique    <boost::multi_index::tag<index::rel_ordered  >, boost::multi_index::identity<relation>, relation::pair_comp>,
				boost::multi_index::hashed_unique     <boost::multi_index::tag<index::rel_unordered>, boost::multi_index::identity<relation>>
			>
		>;

	  private:
		bool running = false;

		std::unordered_map<std::string, std::pair<state_initializer, state_initializer>> states;
		std::vector<state> stack;

		std::map<std::type_index, weak_ref> tagged_objects;

		std::queue<weak_ref> objects_to_remove;
		std::queue<std::pair<weak_ref, std::type_index>> components_to_remove;
		std::queue<std::pair<weak_ref, std::type_index>> components_to_notify;

		std::shared_ptr<component::base> get(weak_ref, std::type_index);

		void remove_now(weak_ref, std::type_index);
		void remove(weak_ref r, std::type_index ti) { components_to_remove.emplace(r, ti); }

	  public:
		bimap objects;

		std::string transition;
		std::unordered_set<std::string> arguments;

		polar(std::vector<std::string> args);

		~polar() {
			// release stack in reverse order
			while(!stack.empty()) { stack.pop_back(); }
		}

		// states

		inline void add(const std::string &name, const state_initializer &init,
		                const state_initializer &destroy = [](polar *, state &) {}) {
			states.emplace(name, std::make_pair(init, destroy));
		}

		void run(const std::string &initialState);

		inline void quit() { running = false; }

		// systems

		void insert(std::type_index, std::shared_ptr<system::base>);
		std::weak_ptr<system::base> get(std::type_index ti);

		template<typename T, typename = typename std::enable_if<std::is_base_of<system::base, T>::value>::type>
		inline std::weak_ptr<T> get() {
			return std::static_pointer_cast<T, system::base>(get(typeid(T)).lock());
		}

		// objects

		ref add();
		void remove_now(weak_ref);
		void remove(weak_ref r) { objects_to_remove.emplace(r); }

		template<
			typename T,
			typename = typename std::enable_if<std::is_base_of<tag::base, T>::value>::type
		> inline ref own() {
			std::type_index ti = typeid(T);
			auto it = tagged_objects.find(ti);
			if(it == tagged_objects.end()) {
				auto r    = ref(std::make_shared<destructor>());
				auto weak = weak_ref(r);
				r.dtor()->set([this, ti, weak] {
					tagged_objects.erase(ti);
					remove(weak);
				});

				tagged_objects.emplace(ti, r);
				return r;
			} else {
				return it->second.own();
			}
		}

		// components

		std::shared_ptr<component::base> insert(weak_ref, std::shared_ptr<component::base>, std::type_index);

		template<typename T, typename... Ts,
		         typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline std::shared_ptr<T> add(weak_ref object, Ts &&... args) {
			return add_as<T, T>(object, std::forward<Ts>(args)...);
		}

		template<typename B, typename T, typename... Ts,
		         typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type,
		         typename = typename std::enable_if<std::is_base_of<B, T>::value>::type>
		inline std::shared_ptr<B> add_as(weak_ref object, Ts &&... args) {
			return insert<B>(object, new T(std::forward<Ts>(args)...));
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline std::shared_ptr<T> insert(weak_ref object, T *component) {
			auto ptr = std::shared_ptr<T>(component);
			return insert(object, ptr);
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline std::shared_ptr<T> insert(weak_ref object, std::shared_ptr<T> component) {
			auto ptr = insert(object, component, typeid(T));
			return std::static_pointer_cast<T>(ptr);
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline std::shared_ptr<const T> get(weak_ref wr) {
			return std::static_pointer_cast<T, component::base>(get(wr, typeid(T)));
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline mutable_component<T> mutate(weak_ref wr) {
			std::type_index ti = typeid(T);
			auto base = get(wr, ti);
			auto ptr = std::static_pointer_cast<T>(base);

			auto mc = mutable_component<T>(ptr, [this, wr, ti, base] {
				for(auto &state : stack) {
					state.mutate(wr, ti, base);
				}
			});

			return mc;
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline void remove(weak_ref object) {
			remove(object, typeid(T));
		}
	};
} // namespace polar::core

#endif
