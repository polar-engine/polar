#pragma once

#include <stdint.h>

namespace polar {
	template<typename T> struct prioritized {
		T value;
		int_fast32_t priority;

		prioritized(T value, int_fast32_t priority = 0) : value(value), priority(priority) {}

		friend inline bool operator<(const prioritized &lhs, const prioritized &rhs) {
			return lhs.value < rhs.value || lhs.priority || rhs.priority;
		}

		friend inline bool operator==(const prioritized &lhs, const prioritized &rhs) {
			return lhs.value == rhs.value;
		}
	};

	template<typename T> inline size_t hash_value(const prioritized<T> &p) {
		boost::hash<T> hasher;
		return hasher(p.value);
	}
} // namespace polar

namespace std {
	template<typename T> struct hash<polar::prioritized<T>> {
		inline size_t operator()(const polar::prioritized<T> &p) const {
			return std::hash<T>()(p.value);
		}
	};
} // namespace std
