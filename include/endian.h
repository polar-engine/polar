#pragma once

template<typename T> inline T swapbe(T n) {
	T r = 0;
	auto data = reinterpret_cast<unsigned char *>(&n);
	for(unsigned int i = 0; i < sizeof(T); ++i) {
		r |= data[sizeof(T) - i - 1] << (i * 8);
	}
	return r;
}

template<typename T> inline T swaple(T n) {
	T r = 0;
	auto data = reinterpret_cast<unsigned char *>(&n);
	for(unsigned int i = 0; i < sizeof(T); ++i) {
		r |= data[i] << (i * 8);
	}
	return r;
}
