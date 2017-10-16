#pragma once

#ifndef POLAR_H
#define POLAR_H

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <polar/component/base.h>
#include <polar/core/debugmanager.h>
#include <polar/core/stack.h>
#include <polar/core/types.h>
#include <polar/util/buildinfo.h>
#include <unordered_map>
#include <vector>

// define types in explicit order to stop loops

namespace polar {
namespace core {
	class polar;
}
}

#include <polar/system/base.h>

#include <polar/core/state.h>

namespace polar {
namespace core {
	class polar {
	  public:
		using priority_t        = support::debug::priority;
		using state_initializer = std::function<void(polar *, state &)>;
		using bimap             = boost::bimap<
		    boost::bimaps::multiset_of<IDType>,
		    boost::bimaps::unordered_multiset_of<const std::type_info *>,
		    boost::bimaps::set_of_relation<>,
		    boost::bimaps::with_info<std::shared_ptr<component::base>>>;

	  private:
		bool initDone = false;
		bool running  = false;
		std::unordered_map<std::string,
		                   std::pair<state_initializer, state_initializer>>
		    states;
		std::vector<state> stack;

		std::weak_ptr<system::base> get(const std::type_info *ti);
		component::base *get(IDType id, const std::type_info *ti);
		void insert(IDType id, std::shared_ptr<component::base> component,
		            const std::type_info *ti);

	  public:
		bimap objects;
		IDType nextID = 1;
		std::string transition;

		polar(std::vector<std::string> args);

		~polar() {
			/* release stack in reverse order */
			while(!stack.empty()) { stack.pop_back(); }
		}

		inline void
		addstate(const std::string &name, const state_initializer &init,
		         const state_initializer &destroy = [](polar *, state &) {}) {
			states.emplace(name, std::make_pair(init, destroy));
		}

		void run(const std::string &initialState);

		inline void quit() { running = false; }

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<system::base, T>::value>::type>
		inline std::weak_ptr<T> get() {
			return std::static_pointer_cast<T, system::base>(
			    get(&typeid(T)).lock());
		}

		std::shared_ptr<destructor> add(IDType *inputID);

		void remove(IDType id);

		template <typename T, typename... Ts,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline void add(IDType id, Ts &&... args) {
			add_as<T, T>(id, std::forward<Ts>(args)...);
		}

		template <typename B, typename T, typename... Ts,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type,
		          typename = typename std::enable_if<
		              std::is_base_of<B, T>::value>::type>
		inline void add_as(IDType id, Ts &&... args) {
			insert<B>(id, new T(std::forward<Ts>(args)...));
		}

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline void insert(IDType id, T *component) {
			insert(id, std::shared_ptr<T>(component));
		}

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline void insert(IDType id, std::shared_ptr<T> component) {
			insert(id, component, &typeid(T));
		}

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline T *get(IDType id) {
			return static_cast<T *>(get(id, &typeid(T)));
		}
	};
}
}

#endif
