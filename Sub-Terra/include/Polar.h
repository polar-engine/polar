#pragma once

#include <boost/weak_ptr.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include "Component.h"
#include "System.h"
#include "EngineState.h"

enum class StackActionType {
	Push,
	Pop
};

struct StackAction {
	StackActionType type;
	std::string name;
};

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
	boost::unordered_map<std::string, StateInitializer> states;
	boost::container::vector<EngineState> stack;
	boost::container::deque<StackAction> actions;
public:
	Bimap objects;
	IDType nextID = 1;

	~Polar() {
		/* release stack in reverse order */
		while(!stack.empty()) {
			stack.pop_back();
		}
	}

	inline void AddState(const std::string &name, const StateInitializer &fn) {
		states.emplace(name, fn);
	}

	inline void PushState(const std::string &name) {
		actions.emplace_back(StackAction{StackActionType::Push, name});
	}

	inline void PopState() {
		actions.emplace_back(StackAction{StackActionType::Pop});
	}

	inline void Run() {
		running = true;
		std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), then;
		while(running) {
			then = now;
			now = std::chrono::high_resolution_clock::now();
			DeltaTicks dt = std::chrono::duration_cast<DeltaTicksBase>(now - then);
			for(auto &state : stack) {
				state.Update(dt);
			}

			/* perform stack actions at end of iteration to avoid invalidation */
			while(!actions.empty()) {
				auto action = actions.front();
				actions.pop_front();
				switch(action.type) {
				case StackActionType::Push:
					INFOS("pushing " << action.name);
					stack.emplace_back(this);
					states[action.name](this, stack.back());
					stack.back().Init();
					break;
				case StackActionType::Pop:
					INFO("popping");
					stack.pop_back();
					break;
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

	inline IDType AddObject() { return nextID++; }

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

	template<typename T> inline void InsertComponent(IDType id, T *component) {
		auto ti = &typeid(T);
		std::shared_ptr<Component> ptr(component);
		objects.insert(Bimap::value_type(id, ti, ptr));
		for(auto &state : stack) {
			state.ComponentAdded(id, ti, ptr);
		}
	}

	template<typename T> inline T * GetComponent(IDType id) {
		auto it = objects.find(Bimap::relation(id, &typeid(T)));
		if(it != objects.end()) {
			return static_cast<T *>(it->info.get());
		} else { return nullptr; }
	}
};
