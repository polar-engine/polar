#include "common.h"
#include "InputManager.h"

void InputManager::Init() {
	auto eventM = engine->systems.Get<EventManager>();
	eventM->ListenFor("keydown", [this] (Arg arg) {
		auto key = *arg.Get<Key>();

		auto range = onKeyHandlers.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->second(key);
		}

		if(std::find(keys.begin(), keys.end(), key) == keys.end()) {
			keys.emplace_back(key);
		}
	});
	eventM->ListenFor("keyup", [this] (Arg arg) {
		keys.erase(std::remove(keys.begin(), keys.end(), *arg.Get<Key>()), keys.end());
	});
	eventM->ListenFor("mousemove", [this] (Arg arg) {
		auto &delta = *arg.Get<Point2>();
		for(auto handler : mouseMoveHandlers) {
			handler(delta);
		}
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
