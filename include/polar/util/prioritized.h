#pragma once

#include <stdint.h>

namespace polar {
	using priority_t = int_fast32_t;

	template<typename T> struct prioritized {
		T value;
		priority_t priority;

		prioritized(T value, priority_t priority = 0) : value(value), priority(priority) {}

		friend inline bool operator==(const prioritized &lhs, const prioritized &rhs) {
			return lhs.value == rhs.value;
		}

		friend inline bool operator<(const prioritized &lhs, const prioritized &rhs) {
			//return lhs.value < rhs.value;
			return lhs.priority < rhs.priority;
			//return lhs.value < rhs.value || lhs.priority < rhs.priority;
		}
	};

	template<typename T> inline size_t hash_value(const prioritized<T> &p) {
		boost::hash<T> hasher;
		return hasher(p.value);
	}

	template<typename T>
	struct priority_comp {
		inline bool operator()(prioritized<T> lhs, prioritized<T> rhs) const {
			return lhs.priority < rhs.priority;
		}
	};
} // namespace polar

namespace std {
	template<typename T> struct hash<polar::prioritized<T>> {
		inline size_t operator()(const polar::prioritized<T> &p) const {
			return std::hash<T>()(p.value);
		}
	};
} // namespace std
