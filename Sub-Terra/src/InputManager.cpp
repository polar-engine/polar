#include "common.h"
#include "InputManager.h"

void InputManager::Init() {
	engine->systems.Get<EventManager>()->ListenFor("keydown", [this] (Arg arg) {
		auto key = *arg.Get<Key>();

		auto range = onKeyHandlers.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->second(key);
		}

		if(std::find(keys.begin(), keys.end(), key) == keys.end()) {
			keys.emplace_back(key);
		}
	});
	engine->systems.Get<EventManager>()->ListenFor("keyup", [this] (Arg arg) {
		keys.erase(std::remove(keys.begin(), keys.end(), *arg.Get<Key>()), keys.end());
	});
}

void InputManager::Update(DeltaTicks &dt, std::vector<Object *> &) {
	for(auto key : keys) {
		auto range = whenKeyHandlers.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->second(key, dt);
		}
	}
}
