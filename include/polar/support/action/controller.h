#pragma once

#include <polar/system/action.h>

namespace polar::support::action::controller {
	struct motion_x : system::action::analog {};
	struct motion_y : system::action::analog {};
}
