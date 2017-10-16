#pragma once

namespace polar {
namespace support {
	namespace debug {
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
}
}
