#pragma once

#include <map>
#include <polar/core/ref.h>

namespace polar::support::action {
	struct digital {};
	struct analog {};

	using digital_function_t = std::function<void(core::weak_ref)>;
	using analog_function_t  = std::function<void(core::weak_ref, math::decimal)>;
	using analog_predicate_t = std::function<bool(core::weak_ref, math::decimal, math::decimal)>;
	using digital_cont_t     = std::function<bool(core::weak_ref)>;
	using analog_cont_t      = std::function<bool(core::weak_ref, math::decimal)>;

	struct digital_data {
		std::map<core::ref, bool> states;
	};

	struct analog_state {
		math::decimal initial  = 0;
		math::decimal previous = 0;
		math::decimal value    = 0;
		math::decimal saved    = 0;
	};

	struct analog_data {
		std::map<core::ref, analog_state> states;
	};

	using digital_map = std::map<std::type_index, digital_data>;
	using analog_map  = std::map<std::type_index, analog_data>;
} // namespace polar::support::action
