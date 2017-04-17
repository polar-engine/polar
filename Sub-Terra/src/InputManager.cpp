#include "common.h"
#include "InputManager.h"
#include "EventManager.h"

void InputManager::Init() {
	auto eventM = engine->GetSystem<EventManager>().lock();
	dtors.emplace_back(eventM->ListenFor("keydown", [this] (Arg arg) {
		auto key = *arg.Get<Key>();

		auto range = onKeyHandlers.left.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->info(key);
		}

		keys.emplace(key);
	}));
	dtors.emplace_back(eventM->ListenFor("keyup", [this] (Arg arg) {
		auto key = *arg.Get<Key>();

		keys.erase(key);

		auto range = afterKeyHandlers.left.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->info(key);
		}
	}));
	dtors.emplace_back(eventM->ListenFor("mousemove", [this] (Arg arg) {
		auto &delta = *arg.Get<Point2>();
		for(auto &handler : mouseMoveHandlers) {
			handler.right(delta);
		}
	}));
	dtors.emplace_back(eventM->ListenFor("mousewheel", [this] (Arg arg) {
		auto &delta = *arg.Get<Point2>();
		for(auto &handler : mouseWheelHandlers) {
			handler.right(delta);
		}
	}));
	dtors.emplace_back(eventM->ListenFor("controlleraxisx", [this] (Arg arg) {
		controllerAxes.x = arg.float_;
	}));
	dtors.emplace_back(eventM->ListenFor("controlleraxisy", [this] (Arg arg) {
		controllerAxes.y = arg.float_;
	}));
}

void InputManager::Update(DeltaTicks &dt) {
	for(auto key : keys) {
		auto range = whenKeyHandlers.left.equal_range(key);
		for(auto it = range.first; it != range.second; ++it) {
			it->info(key, dt);
		}
	}

	auto normalized = controllerAxes / Decimal(32768);
	auto logarithmic = normalized * glm::abs(normalized);

	for(auto &handler : controllerAxesHandlers) {
		handler.right(glm::length(logarithmic) > controllerDeadZone ? logarithmic : Point2(0));
	}
}
