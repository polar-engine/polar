#pragma once

#include <polar/system/action.h>

namespace polar::support::action::mouse {
	struct motion_x : system::action::analog {};
	struct motion_y : system::action::analog {};

	struct wheel_x : system::action::analog {};
	struct wheel_y : system::action::analog {};
}
