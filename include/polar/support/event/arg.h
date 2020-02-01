#pragma once

#include <polar/math/types.h>

namespace polar::support::event {
	union arg {
		math::decimal float_;
		void *pVoid;

		arg(math::decimal f) { float_ = f; }
		arg(std::nullptr_t) { pVoid = nullptr; }
		template<typename T> arg(T *p) { pVoid = reinterpret_cast<void *>(p); }
		template<typename T> inline T *get() {
			return reinterpret_cast<T *>(pVoid);
		}
	};
} // namespace polar::support::event
