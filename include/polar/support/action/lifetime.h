#pragma once

namespace polar::support::action {
	enum class lifetime {
		on,     // instantaneous 0 to 1
		after,  // instantaneous 1 to 0
		when,   // continuous 1
		unless, // continuous 0
		SIZE
	};

	inline std::ostream & operator<<(std::ostream &os, const lifetime &lt) {
		os << "lifetime::";
		switch(lt) {
		default:
			return os << "UNDEFINED";
		case lifetime::on:
			return os << "on";
		case lifetime::after:
			return os << "after";
		case lifetime::when:
			return os << "when";
		case lifetime::unless:
			return os << "unless";
		}
	}
}
