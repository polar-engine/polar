#include <polar/core/polar.h>
#include <polar/system/input.h>
#include <polar/system/event.h>

namespace polar { namespace system {
	void input::init() {
		auto eventM = engine->getsystem<event>().lock();
		dtors.emplace_back(eventM->listenfor("keydown", [this] (arg_t arg) {
			auto key = *arg.get<key_t>();

			auto range = onKeyHandlers.left.equal_range(key);
			for(auto it = range.first; it != range.second; ++it) {
				it->info(key);
			}

			keys.emplace(key);
		}));
		dtors.emplace_back(eventM->listenfor("keyup", [this] (arg_t arg) {
			auto key = *arg.get<key_t>();

			keys.erase(key);

			auto range = afterKeyHandlers.left.equal_range(key);
			for(auto it = range.first; it != range.second; ++it) {
				it->info(key);
			}
		}));
		dtors.emplace_back(eventM->listenfor("mousemove", [this] (arg_t arg) {
			auto &delta = *arg.get<Point2>();
			for(auto &handler : mouseMoveHandlers) {
				handler.right(delta);
			}
		}));
		dtors.emplace_back(eventM->listenfor("mousewheel", [this] (arg_t arg) {
			auto &delta = *arg.get<Point2>();
			for(auto &handler : mouseWheelHandlers) {
				handler.right(delta);
			}
		}));
		dtors.emplace_back(eventM->listenfor("controlleraxisx", [this] (arg_t arg) {
			controllerAxes.x = arg.float_;
		}));
		dtors.emplace_back(eventM->listenfor("controlleraxisy", [this] (arg_t arg) {
			controllerAxes.y = arg.float_;
		}));
	}

	void input::update(DeltaTicks &dt) {
		if(currentSetAccum > 0) { currentSetAccum -= dt.Seconds(); }

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

		ControllerHandle_t cs[STEAM_CONTROLLER_MAX_COUNT];
		debugmanager()->trace("GetConnectedControllers before");
		int numControllers = SteamController()->GetConnectedControllers(cs);
		debugmanager()->trace("GetConnectedControllers after");

		/*auto con = SteamController();
		ControllerActionSetHandle_t setHandle = con->GetActionSetHandle("MenuControls");
		ControllerDigitalActionHandle_t digital = con->GetDigitalActionHandle("menu_down");
		ControllerAnalogActionHandle_t analog = con->GetAnalogActionHandle("menu_down");
		INFOS(digital << ' ' << analog);
		for(int i = 0; i < numControllers; ++i) {
			con->ActivateActionSet(cs[i], setHandle);
			ControllerDigitalActionData_t data = con->GetDigitalActionData(cs[i], digital);
			INFOS(data.bState);
			EControllerActionOrigin os[STEAM_CONTROLLER_MAX_ORIGINS];
			int numOrigins = con->GetDigitalActionOrigins(cs[i], setHandle, digital, os);
			for(int o = 0; o < numOrigins; ++o) {
				INFOS(con->GetStringForActionOrigin(os[o]));
				INFOS(con->GetGlyphForActionOrigin(os[o]));
			}
		}*/

		for(int i = 0; i < numControllers; ++i) {
			debugmanager()->trace("ActivateActionSet before");
			SteamController()->ActivateActionSet(cs[i], currentActionSet);
			debugmanager()->trace("ActivateActionSet after");
		}

		for(auto digital : trackedDigitals) {
			bool active = false;
			for(int i = 0; i < numControllers; ++i) {
				debugmanager()->trace("GetDigitalActionData before");
				ControllerDigitalActionData_t data = SteamController()->GetDigitalActionData(cs[i], digital);
				debugmanager()->trace("GetDigitalActionData after");
				active |= data.bActive && data.bState;
			}
			if(active) {
				// currentSetAccum avoids phantom activations after switching action set
				if(currentSetAccum <= 0 && digitals.find(digital) == digitals.cend()) {
					auto range = onDigitalHandlers.left.equal_range(digital);
					for(auto it = range.first; it != range.second; ++it) {
						it->info();
					}
				}
				digitals.emplace(digital);
			} else {
				digitals.erase(digital);
			}
		}

		for(auto analog : trackedAnalogs) {
			Point2 delta(0);
			for(int i = 0; i < numControllers; ++i) {
				debugmanager()->trace("GetAnalogActionData before");
				ControllerAnalogActionData_t data = SteamController()->GetAnalogActionData(cs[i], analog);
				debugmanager()->trace("GetAnalogActionData after");
				delta.x += data.x;
				delta.y += data.y;
			}
			if(currentSetAccum <= 0) {
				auto range = onAnalogHandlers.left.equal_range(analog);
				for(auto it = range.first; it != range.second; ++it) {
					it->info(delta);
				}
			}
		}
	}
} }
