#include <polar/core/polar.h>
#include <polar/system/event.h>
#include <polar/system/input.h>

namespace polar {
namespace system {
	void input::init() {
		auto eventM = engine->get_system<event>().lock();
		dtors.emplace_back(eventM->listenfor("keydown", [this](arg_t arg) {
			auto key = *arg.get<key_t>();

			auto range = onKeyHandlers.left.equal_range(key);
			for(auto it = range.first; it != range.second; ++it) {
				it->info(key);
			}

			keys.emplace(key);
		}));
		dtors.emplace_back(eventM->listenfor("keyup", [this](arg_t arg) {
			auto key = *arg.get<key_t>();

			keys.erase(key);

			auto range = afterKeyHandlers.left.equal_range(key);
			for(auto it = range.first; it != range.second; ++it) {
				it->info(key);
			}
		}));
		dtors.emplace_back(eventM->listenfor("mousemove", [this](arg_t arg) {
			auto &delta = *arg.get<Point2>();
			for(auto &handler : mouseMoveHandlers) { handler.right(delta); }
		}));
		dtors.emplace_back(eventM->listenfor("mousewheel", [this](arg_t arg) {
			auto &delta = *arg.get<Point2>();
			for(auto &handler : mouseWheelHandlers) { handler.right(delta); }
		}));
		dtors.emplace_back(
		    eventM->listenfor("controlleraxisx", [this](arg_t arg) {
			    controllerAxes.x = arg.float_;
			}));
		dtors.emplace_back(
		    eventM->listenfor("controlleraxisy", [this](arg_t arg) {
			    controllerAxes.y = arg.float_;
			}));
	}

	void input::update(DeltaTicks &dt) {
		for(auto key : keys) {
			auto range = whenKeyHandlers.left.equal_range(key);
			for(auto it = range.first; it != range.second; ++it) {
				it->info(key, dt);
			}
		}

		auto normalized  = controllerAxes / Decimal(32768);
		auto logarithmic = normalized * glm::abs(normalized);

		for(auto &handler : controllerAxesHandlers) {
			handler.right(glm::length(logarithmic) > controllerDeadZone
			                  ? logarithmic
			                  : Point2(0));
		}
	}
}
}
