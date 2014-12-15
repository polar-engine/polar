#pragma once

#ifdef _WIN32
#include <Windows.h>

template<typename T> class Atomic {
private:
	T value;
	CRITICAL_SECTION critSect;
public:
	template<typename ...Ts> Atomic(Ts && ...args) : value(std::forward<Ts>(args)...) {
		InitializeCriticalSection(&critSect);
	}

	~Atomic() {
		DeleteCriticalSection(&critSect);
	}

	inline void With(const std::function<void(T &)> &fn) {
		EnterCriticalSection(&critSect);
		fn(value);
		LeaveCriticalSection(&critSect);
	}

	template<typename Ret> inline Ret With(const std::function<Ret(T &)> &fn) {
		EnterCriticalSection(&critSect);
		Ret ret = fn(value);
		LeaveCriticalSection(&critSect);
		return ret;
	}
};

#else

template<typename T> class Atomic {
private:
	T value;
	std::recursive_mutex mutex;
public:
	template<typename ...Ts> Atomic(Ts && ...args) : value(std::forward<Ts>(args)...) {}

	inline void With(const std::function<void(T &)> &fn) {
		std::lock_guard<Mutex> lock(mutex);
		fn(value);
	}

	template<typename Ret> inline Ret With(const std::function<Ret(T &)> &fn) {
		std::lock_guard<Mutex> lock(mutex);
		return fn(value);
	}
};

#endif
