#pragma once

template<typename T> inline T swapbe(T n) {
	T r       = 0;
	auto data = reinterpret_cast<unsigned char *>(&n);
	for(size_t i = 0; i < sizeof(T); ++i) {
		r |= T(data[sizeof(T) - i - 1]) << T(i * 8);
	}
	return r;
}

template<typename T> inline T swaple(T n) {
	T r       = 0;
	auto data = reinterpret_cast<unsigned char *>(&n);
	for(size_t i = 0; i < sizeof(T); ++i) { r |= T(data[i]) << T(i * 8); }
	return r;
}
