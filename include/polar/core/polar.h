#pragma once

#ifndef POLAR_H
#define POLAR_H

//#include <boost/bimap.hpp>
//#include <boost/bimap/multiset_of.hpp>
//#include <boost/bimap/set_of.hpp>
//#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <polar/component/base.h>
#include <polar/core/log.h>
#include <polar/core/stack.h>
#include <polar/math/types.h>
#include <polar/util/buildinfo.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// define types in explicit order to stop loops

namespace polar::core {
	class polar;
}

#include <polar/system/base.h>

#include <polar/core/state.h>

namespace polar::core {
	namespace index {
		struct ref  {};
		struct ti   {};
		struct pair {};
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

			/*
			inline bool operator()(const relation &lhs, const std::type_index &rhs) const {
				return lhs.ti < rhs;
			}

			inline bool operator()(const std::type_index &lhs, const relation &rhs) const {
				return lhs < rhs.ti;
			}
			*/
		};

		weak_ref r;
		std::type_index ti;
		std::shared_ptr<component::base> ptr;
	};

	class polar {
	  public:
		using priority_t        = support::debug::priority;
		using state_initializer = std::function<void(polar *, state &)>;

		/*
		using bimap =
		    boost::bimap<boost::bimaps::multiset_of<weak_ref>, boost::bimaps::multiset_of<std::type_index>,
		                 boost::bimaps::set_of_relation<>, boost::bimaps::with_info<std::shared_ptr<component::base>>>;
		*/

		using bimap = boost::multi_index_container<
			relation,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_non_unique<boost::multi_index::tag<index::ref >, boost::multi_index::member<relation, weak_ref,        &relation::r>>,
				boost::multi_index::ordered_non_unique<boost::multi_index::tag<index::ti  >, boost::multi_index::member<relation, std::type_index, &relation::ti>>,
				boost::multi_index::ordered_unique    <boost::multi_index::tag<index::pair>, boost::multi_index::identity<relation>, relation::pair_comp>
			>
		>;

	  private:
		bool initDone = false;
		bool running  = false;
		std::unordered_map<std::string, std::pair<state_initializer, state_initializer>> states;
		std::vector<state> stack;

		std::vector<weak_ref> objects_to_remove;
		std::vector<std::pair<weak_ref, std::type_index>> components_to_remove;

		component::base *get(weak_ref, std::type_index);
		void insert(weak_ref, std::shared_ptr<component::base>, std::type_index);
		void remove_now(weak_ref, std::type_index);

		void remove(weak_ref r, std::type_index ti) { components_to_remove.emplace_back(r, ti); }

	  public:
		bimap objects;
		std::string transition;

		std::unordered_set<std::string> arguments;

		polar(std::vector<std::string> args);

		~polar() {
			/* release stack in reverse order */
			while(!stack.empty()) { stack.pop_back(); }
		}

		std::weak_ptr<system::base> get(std::type_index ti);

		inline void add(
		    const std::string &name, const state_initializer &init,
		    const state_initializer &destroy = [](polar *, state &) {}) {
			states.emplace(name, std::make_pair(init, destroy));
		}

		void run(const std::string &initialState);

		inline void quit() { running = false; }

		template<typename T, typename = typename std::enable_if<std::is_base_of<system::base, T>::value>::type>
		inline std::weak_ptr<T> get() {
			return std::static_pointer_cast<T, system::base>(get(typeid(T)).lock());
		}

		ref add();
		void insert(std::type_index, std::shared_ptr<system::base>);
		void remove_now(weak_ref);

		void remove(weak_ref r) { objects_to_remove.emplace_back(r); }

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
			insert(object, ptr);
			return ptr;
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline std::shared_ptr<T> insert(weak_ref object, std::shared_ptr<T> component) {
			insert(object, component, typeid(T));
			return component;
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline T *get(weak_ref object) {
			return static_cast<T *>(get(object, typeid(T)));
		}

		template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
		inline void remove(weak_ref object) {
			remove(object, typeid(T));
		}
	};
} // namespace polar::core

#endif
