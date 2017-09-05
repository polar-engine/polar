#pragma once

namespace polar { namespace support { namespace event {
	union arg {
		Decimal float_;
		void *pVoid;

		arg(Decimal f) { float_ = f; }
		arg(std::nullptr_t) { pVoid = nullptr; }
		template<typename T> arg(T *p) { pVoid = reinterpret_cast<void *>(p); }
		template<typename T> inline T * get() { return reinterpret_cast<T *>(pVoid); }
	};
} } }
