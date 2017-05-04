#pragma once

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <steam/steam_api.h>
#include "DebugManager.h"
#include "Component.h"
#include "System.h"
#include "EngineStack.h"
#include "EngineState.h"

class Polar {
public:
	typedef std::function<void(Polar *, EngineState &)> StateInitializer;
	typedef boost::bimap<
		boost::bimaps::multiset_of<IDType>,
		boost::bimaps::unordered_multiset_of<const std::type_info *>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<boost::shared_ptr<Component>>
	> Bimap;
private:
	bool initDone = false;
	bool running = false;
	boost::unordered_map<std::string, std::pair<StateInitializer, StateInitializer>> states;
	boost::container::vector<EngineState> stack;
public:
	DebugManager dm;
	Bimap objects;
	IDType nextID = 1;
	std::string transition;

	Polar(DebugManager::Priority priority = DebugManager::Priority::Info) : dm(priority) {
		if(!SteamAPI_Init()) {
			ENGINE_THROW("failed to initialize Steam API");
		}
		dm.Info("Welcome, ", SteamFriends()->GetPersonaName());
		SteamController()->Init();
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
		while(running) {
			SteamAPI_RunCallbacks();

			then = now;
			now = std::chrono::high_resolution_clock::now();
			DeltaTicks dt = std::chrono::duration_cast<DeltaTicksBase>(now - then);

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
						stack.emplace_back(action.name, this);
						states[action.name].first(this, stack.back());
						stack.back().Init();
						break;
					case StackActionType::Pop: {
						auto &state = stack.back();
						states[state.name].second(this, state);
						stack.pop_back();
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

	template<typename T> inline boost::weak_ptr<T> GetSystem() {
		for(auto &state : stack) {
			auto ptr = state.GetSystem<T>();
			if(!ptr.expired()) { return ptr; }
		}
		return boost::weak_ptr<T>();
	}

	inline boost::shared_ptr<Destructor> AddObject(IDType *inputID) {
		auto id = nextID++;
		*inputID = id;
		return boost::make_shared<Destructor>([this, id] () {
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
		InsertComponent(id, boost::shared_ptr<T>(component));
	}

	template<typename T> inline void InsertComponent(IDType id, boost::shared_ptr<T> component) {
		auto ti = &typeid(T);
		objects.insert(Bimap::value_type(id, ti, component));
		for(auto &state : stack) {
			state.ComponentAdded(id, ti, component);
		}
	}

	template<typename T> inline T * GetComponent(IDType id) {
		auto it = objects.find(Bimap::relation(id, &typeid(T)));
		if(it != objects.end()) {
			return static_cast<T *>(it->info.get());
		} else { return nullptr; }
	}
};
