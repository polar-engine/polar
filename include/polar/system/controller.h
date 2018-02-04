#pragma once

#include <array>
#include <polar/system/action.h>

namespace polar::system {
	class controller : public system::base {
	private:
		action::analog_ref a_motion_x;
		action::analog_ref a_motion_y;
	public:
		static bool supported() { return true; }
		controller(core::polar *engine) : base(engine) {}

		void init() final {
			auto act = engine->get<system::action>().lock();

			a_motion_x = act->analog();
			a_motion_y = act->analog();
		}

		const auto action_motion_x() { return a_motion_x; }
		const auto action_motion_y() { return a_motion_y; }
	};
}
