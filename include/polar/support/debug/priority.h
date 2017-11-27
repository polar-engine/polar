#pragma once

namespace polar::support::debug {
	enum class priority : uint_fast8_t {
		trace,
		debug,
		verbose,
		info,
		notice,
		warning,
		error,
		critical,
		fatal,
		_size
	};
}
