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

class Polar {
public:
	typedef std::function<void(Polar *, EngineState &)> StateInitializer;
	typedef boost::bimap<
		boost::bimaps::multiset_of<IDType>,
		boost::bimaps::unordered_multiset_of<const std::type_info *>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<std::shared_ptr<Component>>
	> Bimap;
private:
	bool initDone = false;
	bool running = false;
	std::unordered_map<std::string, std::pair<StateInitializer, StateInitializer>> states;
	std::vector<EngineState> stack;
public:
	Bimap objects;
	IDType nextID = 1;
	std::string transition;

	Polar(std::vector<std::string> args) {
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
				DebugManager()->priority = DebugPriority::Trace;
			} else if(arg == "-debug") {
				DebugManager()->priority = DebugPriority::Debug;
			} else if(arg == "-verbose") {
				DebugManager()->priority = DebugPriority::Verbose;
			}
		}

		DebugManager()->Verbose("built on ", buildinfo_date(), " at ", buildinfo_time());

		if(!SteamAPI_Init()) {
			DebugManager()->Fatal("failed to initialize Steam API");
		}
		DebugManager()->Info("Welcome, ", SteamFriends()->GetPersonaName());
		SteamController()->Init();
		SteamUserStats()->RequestCurrentStats();
	}

	~Polar() {
		/* release stack in reverse order */
		while(!stack.empty()) {
			stack.pop_back();
		}

		SteamController()->Shutdown();
		SteamAPI_Shutdown();
	}

	inline void AddState(const std::string &name,
						 const StateInitializer &init,
						 const StateInitializer &destroy = [] (Polar *, EngineState &) {}) {
		states.emplace(name, std::make_pair(init, destroy));
	}

	inline void Run(const std::string &initialState) {
		running = true;

		stack.emplace_back(initialState, this);
		states[initialState].first(this, stack.back());
		stack.back().Init();

		std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), then;

		uint64_t frameID = 0;
		while(running) {
			then = now;
			now = std::chrono::high_resolution_clock::now();
			DeltaTicks dt = std::chrono::duration_cast<DeltaTicksBase>(now - then);

			DebugManager()->Trace("frame #", frameID++, " (", dt.Ticks(), ')');

			DebugManager()->Trace("SteamAPI_RunCallbacks before");
			SteamAPI_RunCallbacks();
			DebugManager()->Trace("SteamAPI_RunCallbacks after");

			for(auto &state : stack) {
				state.Update(dt);
			}

			/* perform transition at end of iteration to avoid invalidation */
			if(transition != "") {
				auto actions = stack.back().transitions[transition];
				transition = "";
				for(auto &action : actions) {
					switch(action.type) {
					case StackActionType::Push:
						DebugManager()->Debug("pushing state: ", action.name);
						stack.emplace_back(action.name, this);
						{
							DebugManager()->Debug("calling state initializer");
							EngineState &st = stack.back();
							DebugManager()->Debug("calling state initializer");
							states[action.name].first(this, st);
						}
						DebugManager()->Debug("pushed state");
						stack.back().Init();
						break;
					case StackActionType::Pop: {
						auto &state = stack.back();
						DebugManager()->Debug("popping state: ", state.name);
						states[state.name].second(this, state);
						stack.pop_back();
						DebugManager()->Debug("popped state");
						break;
					}
					case StackActionType::Quit:
						Quit();
						break;
					}
				}
			}
		}
	}


	inline void Quit() {
		running = false;
	}

	template<typename T> inline std::weak_ptr<T> GetSystem() {
		for(auto &state : stack) {
			auto ptr = state.GetSystem<T>();
			if(!ptr.expired()) { return ptr; }
		}
		return std::weak_ptr<T>();
	}

	inline std::shared_ptr<Destructor> AddObject(IDType *inputID) {
		auto id = nextID++;
		*inputID = id;
		return std::make_shared<Destructor>([this, id] () {
			RemoveObject(id);
		});
	}

	inline void RemoveObject(IDType id) {
		auto pairLeft = objects.left.equal_range(id);
		for(auto it = pairLeft.first; it != pairLeft.second; ++it) {
			for(auto &state : stack) {
				state.ComponentRemoved(id, it->get_right());
			}
		}
		objects.left.erase(id);
	}

	template<typename T, typename ...Ts> inline void AddComponent(IDType id, Ts && ...args) {
		InsertComponent(id, new T(std::forward<Ts>(args)...));
	}

	template<typename B, typename T, typename ...Ts> inline void AddComponentAs(IDType id, Ts && ...args) {
		static_assert(std::is_base_of<B, T>::value, "AddComponentAs requires base class and sub class");
		InsertComponent<B>(id, new T(std::forward<Ts>(args)...));
	}

	template<typename T> inline void InsertComponent(IDType id, T *component) {
		InsertComponent(id, std::shared_ptr<T>(component));
	}

	template<typename T> inline void InsertComponent(IDType id, std::shared_ptr<T> component) {
		auto ti = &typeid(T);
		DebugManager()->Trace("inserting component: ", ti->name());
		objects.insert(Bimap::value_type(id, ti, component));
		for(auto &state : stack) {
			state.ComponentAdded(id, ti, component);
		}
		DebugManager()->Trace("inserted component");
	}

	template<typename T> inline T * GetComponent(IDType id) {
		auto it = objects.find(Bimap::relation(id, &typeid(T)));
		if(it != objects.end()) {
			return static_cast<T *>(it->info.get());
		} else { return nullptr; }
	}
};
