#pragma once

#include <polar/system/action.h>

namespace polar::support::action::vr {
	enum class type {
		left_hand,
		right_hand,
		any
	};

	template<type Ty> struct app_menu : system::action::digital {};
	template<type Ty> struct a        : system::action::digital {};
	template<type Ty> struct trigger  : system::action::digital {};
}
