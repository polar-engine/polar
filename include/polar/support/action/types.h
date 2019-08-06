#pragma once

#include <polar/support/action/types.h>

namespace polar::support::action {
	struct digital {};
	struct analog {};

	using digital_function_t = std::function<void(IDType)>;
	using analog_function_t  = std::function<void(IDType, Decimal)>;
	using analog_predicate_t = std::function<bool(IDType, Decimal, Decimal)>;
}
