#pragma once

#include <polar/system/action.h>

namespace polar::support::action::mouse {
	struct position_x : system::action::analog {};
	struct position_y : system::action::analog {};

	struct motion_x : system::action::analog {};
	struct motion_y : system::action::analog {};

	struct wheel_x : system::action::analog {};
	struct wheel_y : system::action::analog {};
}
