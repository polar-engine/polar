#pragma once

#define POLAR_BASE_POLAR_H

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#endif

#include <vector>
#include <deque>
#include <random>
#include <chrono>
#include <unordered_map>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <steam/steam_api.h>
#include <polar/core/types.h>
#include <polar/core/debugmanager.h>
#include <polar/component/base.h>
#include <polar/system/base.h>
#include <polar/core/stack.h>
#include <polar/core/state.h>
#include <polar/util/buildinfo.h>

namespace polar { namespace core {
	class polar {
	public:
		using priority_t = support::debug::priority;
		using state_initializer = std::function<void(polar *, state &)>;
		using bimap = boost::bimap<
			boost::bimaps::multiset_of<IDType>,
			boost::bimaps::unordered_multiset_of<const std::type_info *>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<std::shared_ptr<component::base>>
		>;
	private:
		bool initDone = false;
		bool running = false;
		std::unordered_map<std::string, std::pair<state_initializer, state_initializer>> states;
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

			debugmanager()->verbose("built on ", buildinfo_date(), " at ", buildinfo_time());

			if(!SteamAPI_Init()) {
				debugmanager()->fatal("failed to initialize Steam API");
			}
			debugmanager()->info("Welcome, ", SteamFriends()->GetPersonaName());
			SteamController()->Init();
			SteamUserStats()->RequestCurrentStats();
		}

		~polar() {
			/* release stack in reverse order */
			while(!stack.empty()) {
				stack.pop_back();
			}

			SteamController()->Shutdown();
			SteamAPI_Shutdown();
		}

		inline void addstate(const std::string &name,
							 const state_initializer &init,
							 const state_initializer &destroy = [] (polar *, state &) {}) {
			states.emplace(name, std::make_pair(init, destroy));
		}

		inline void run(const std::string &initialState) {
			running = true;

			stack.emplace_back(initialState, this);
			states[initialState].first(this, stack.back());
			stack.back().init();

			std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), then;

			uint64_t frameID = 0;
			while(running) {
				then = now;
				now = std::chrono::high_resolution_clock::now();
				DeltaTicks dt = std::chrono::duration_cast<DeltaTicksBase>(now - then);

				debugmanager()->trace("frame #", frameID++, " (", dt.Ticks(), ')');

				debugmanager()->trace("SteamAPI_RunCallbacks before");
				SteamAPI_RunCallbacks();
				debugmanager()->trace("SteamAPI_RunCallbacks after");

				for(auto &state : stack) {
					state.update(dt);
				}

				/* perform transition at end of iteration to avoid invalidation */
				if(transition != "") {
					auto actions = stack.back().transitions[transition];
					transition = "";
					for(auto &action : actions) {
						switch(action.type) {
						case StackActionType::Push:
							debugmanager()->debug("pushing state: ", action.name);
							stack.emplace_back(action.name, this);
							{
								debugmanager()->debug("calling state initializer");
								state &st = stack.back();
								debugmanager()->debug("calling state initializer");
								states[action.name].first(this, st);
							}
							debugmanager()->debug("pushed state");
							stack.back().init();
							break;
						case StackActionType::Pop: {
							auto &state = stack.back();
							debugmanager()->debug("popping state: ", state.name);
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


		inline void quit() {
			running = false;
		}

		template<typename T> inline std::weak_ptr<T> getsystem() {
			for(auto &state : stack) {
				auto ptr = state.getsystem<T>();
				if(!ptr.expired()) { return ptr; }
			}
			return std::weak_ptr<T>();
		}

		inline std::shared_ptr<destructor> addobject(IDType *inputID) {
			auto id = nextID++;
			*inputID = id;
			return std::make_shared<destructor>([this, id] () {
				removeobject(id);
			});
		}

		inline void removeobject(IDType id) {
			auto pairLeft = objects.left.equal_range(id);
			for(auto it = pairLeft.first; it != pairLeft.second; ++it) {
				for(auto &state : stack) {
					state.componentremoved(id, it->get_right());
				}
			}
			objects.left.erase(id);
		}

		template<typename T, typename ...Ts> inline void addcomponent(IDType id, Ts && ...args) {
			insertcomponent(id, new T(std::forward<Ts>(args)...));
		}

		template<typename B, typename T, typename ...Ts> inline void addcomponent_as(IDType id, Ts && ...args) {
			static_assert(std::is_base_of<B, T>::value, "addcomponent_as requires base class and sub class");
			insertcomponent<B>(id, new T(std::forward<Ts>(args)...));
		}

		template<typename T> inline void insertcomponent(IDType id, T *component) {
			insertcomponent(id, std::shared_ptr<T>(component));
		}

		template<typename T> inline void insertcomponent(IDType id, std::shared_ptr<T> component) {
			auto ti = &typeid(T);
			debugmanager()->trace("inserting component: ", ti->name());
			objects.insert(Bimap::value_type(id, ti, component));
			for(auto &state : stack) {
				state.componentadded(id, ti, component);
			}
			DebugManager()->Trace("inserted component");
		}

		template<typename T> inline T * getcomponent(IDType id) {
			auto it = objects.find(Bimap::relation(id, &typeid(T)));
			if(it != objects.end()) {
				return static_cast<T *>(it->info.get());
			} else { return nullptr; }
		}
	};
} }
