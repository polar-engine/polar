#pragma once

#include <polar/system/action.h>

namespace polar::support::action::menu {
	struct up       : system::action::digital {};
	struct down     : system::action::digital {};
	struct right    : system::action::digital {};
	struct left     : system::action::digital {};
	struct forward  : system::action::digital {};
	struct back : system::action::digital {};
}
