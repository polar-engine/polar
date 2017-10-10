#pragma once

#define POLAR_BASE_POLAR_H

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#endif

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <chrono>
#include <deque>
#include <polar/component/base.h>
#include <polar/core/debugmanager.h>
#include <polar/core/stack.h>
#include <polar/core/state.h>
#include <polar/core/types.h>
#include <polar/system/base.h>
#include <polar/util/buildinfo.h>
#include <random>
#include <unordered_map>
#include <vector>

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

	  public:
		bimap objects;
		IDType nextID = 1;
		std::string transition;

		polar(std::vector<std::string> args) {
			srand((unsigned int)time(0));
			std::mt19937_64 rng(time(0));

			for(auto &arg : args) {
				if(arg == "-console") {
#if defined(_WIN32)
					AllocConsole();
					freopen("CONIN$", "r", stdin);
					freopen("CONOUT$", "w", stdout);
					freopen("CONOUT$", "w", stderr);
					std::wcout.clear();
					std::cout.clear();
					std::wcerr.clear();
					std::cerr.clear();
					std::wcin.clear();
					std::cin.clear();
#endif
				} else if(arg == "-trace") {
					debugmanager()->priority = priority_t::trace;
				} else if(arg == "-debug") {
					debugmanager()->priority = priority_t::debug;
				} else if(arg == "-verbose") {
					debugmanager()->priority = priority_t::verbose;
				}
			}

			debugmanager()->verbose("built on ", buildinfo_date(), " at ",
			                        buildinfo_time());
		}

		~polar() {
			/* release stack in reverse order */
			while(!stack.empty()) { stack.pop_back(); }
		}

		inline void
		addstate(const std::string &name, const state_initializer &init,
		         const state_initializer &destroy = [](polar *, state &) {}) {
			states.emplace(name, std::make_pair(init, destroy));
		}

		inline void run(const std::string &initialState) {
			running = true;

			stack.emplace_back(initialState, this);
			states[initialState].first(this, stack.back());
			stack.back().init();

			std::chrono::time_point<std::chrono::high_resolution_clock>
			    now = std::chrono::high_resolution_clock::now(),
			    then;

			uint64_t frameID = 0;
			while(running) {
				then = now;
				now  = std::chrono::high_resolution_clock::now();
				DeltaTicks dt =
				    std::chrono::duration_cast<DeltaTicksBase>(now - then);

				debugmanager()->trace("frame #", frameID++, " (", dt.Ticks(),
				                      ')');

				for(auto &state : stack) { state.update(dt); }

				/* perform transition at end of iteration to avoid invalidation
				 */
				if(transition != "") {
					auto actions = stack.back().transitions[transition];
					transition   = "";
					for(auto &action : actions) {
						switch(action.type) {
						case StackActionType::Push:
							debugmanager()->debug("pushing state: ",
							                      action.name);
							stack.emplace_back(action.name, this);
							{
								debugmanager()->debug(
								    "calling state initializer");
								state &st = stack.back();
								debugmanager()->debug(
								    "calling state initializer");
								states[action.name].first(this, st);
							}
							debugmanager()->debug("pushed state");
							stack.back().init();
							break;
						case StackActionType::Pop: {
							auto &state = stack.back();
							debugmanager()->debug("popping state: ",
							                      state.name);
							states[state.name].second(this, state);
							stack.pop_back();
							debugmanager()->debug("popped state");
							break;
						}
						case StackActionType::Quit:
							quit();
							break;
						}
					}
				}
			}
		}

		inline void quit() { running = false; }

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<system::base, T>::value>::type>
		inline std::weak_ptr<T> get_system() {
			for(auto &state : stack) {
				auto ptr = state.get_system<T>();
				if(!ptr.expired()) { return ptr; }
			}
			return std::weak_ptr<T>();
		}

		inline std::shared_ptr<destructor> add_object(IDType *inputID) {
			auto id  = nextID++;
			*inputID = id;
			return std::make_shared<destructor>(
			    [this, id]() { remove_object(id); });
		}

		inline void remove_object(IDType id) {
			auto pairLeft = objects.left.equal_range(id);
			for(auto it = pairLeft.first; it != pairLeft.second; ++it) {
				for(auto &state : stack) {
					state.component_removed(id, it->get_right());
				}
			}
			objects.left.erase(id);
		}

		template <typename T, typename... Ts,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline void add_component(IDType id, Ts &&... args) {
			add_component_as<T, T>(id, std::forward<Ts>(args)...);
		}

		template <typename B, typename T, typename... Ts,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type,
		          typename = typename std::enable_if<
		              std::is_base_of<B, T>::value>::type>
		inline void add_component_as(IDType id, Ts &&... args) {
			insert_component<B>(id, new T(std::forward<Ts>(args)...));
		}

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline void insert_component(IDType id, T *component) {
			insert_component(id, std::shared_ptr<T>(component));
		}

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline void insert_component(IDType id, std::shared_ptr<T> component) {
			auto ti = &typeid(T);
			debugmanager()->trace("inserting component: ", ti->name());
			objects.insert(bimap::value_type(id, ti, component));
			for(auto &state : stack) {
				state.component_added(id, ti, component);
			}
			debugmanager()->trace("inserted component");
		}

		template <typename T,
		          typename = typename std::enable_if<
		              std::is_base_of<component::base, T>::value>::type>
		inline T *get_component(IDType id) {
			auto it = objects.find(bimap::relation(id, &typeid(T)));
			if(it != objects.end()) {
				return static_cast<T *>(it->info.get());
			} else {
				return nullptr;
			}
		}
	};
}
}
