#pragma once

#include <polar/support/action/types.h>

namespace polar::support::action {
	struct digital {};
	struct analog {};

	using digital_function_t = std::function<void(IDType)>;
	using analog_function_t  = std::function<void(IDType, Decimal)>;
	using analog_predicate_t = std::function<bool(IDType, Decimal, Decimal)>;
	using digital_cont_t     = std::function<bool(IDType)>;
	using analog_cont_t      = std::function<bool(IDType, Decimal)>;

	struct digital_data {
		std::unordered_map<IDType, bool> states;
	};

	struct analog_state {
		Decimal initial = 0;
		Decimal previous = 0;
		Decimal value = 0;
		Decimal saved = 0;
	};

	struct analog_data {
		std::unordered_map<IDType, analog_state> states;
	};

	using digital_map = std::unordered_map<std::type_index, digital_data>;
	using analog_map  = std::unordered_map<std::type_index, analog_data>;
}
