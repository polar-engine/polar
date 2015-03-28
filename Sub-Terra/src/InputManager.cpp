#include "common.h"
#include "InputManager.h"
#include "EventManager.h"

void InputManager::Init() {
	auto eventM = engine->systems.Get<EventManager>().lock();
	eventM->ListenFor("keydown", [this] (Arg arg) {
		auto key = *arg.Get<Key>();

		auto range = onKeyHandlers.left.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->info(key);
		}

		keys.emplace(key);
	});
	eventM->ListenFor("keyup", [this] (Arg arg) {
		auto key = *arg.Get<Key>();

		keys.erase(key);

		auto range = afterKeyHandlers.left.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->info(key);
		}
	});
	eventM->ListenFor("mousemove", [this] (Arg arg) {
		auto &delta = *arg.Get<Point2>();
		for(auto handler : mouseMoveHandlers) {
			handler(delta);
		}
	});
}

void InputManager::Update(DeltaTicks &dt) {
	for(auto key : keys) {
		auto range = whenKeyHandlers.left.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->info(key, dt);
		}
	}
}
