#pragma once

/*#ifdef _WIN32
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

#else*/

#include <mutex>
#include <condition_variable>

template<typename T> class Atomic {
public:
	typedef std::mutex mutex_type;
private:
	T value;
	mutex_type mutex;
	std::condition_variable cv;
public:
	template<typename ...Ts> Atomic(Ts && ...args) : value(std::forward<Ts>(args)...) {}

	inline void Notify() {
		cv.notify_one();
	}

	inline void Wait(const std::function<bool(T &)> &pred, const std::function<void(T &)> &fn) {
		std::unique_lock<std::mutex> lock(mutex);
		while(!pred(value)) {
			cv.wait(lock);
		}
		fn(value);
	}

	inline void With(const std::function<void(T &)> &fn) {
		std::lock_guard<mutex_type> lock(mutex);
		fn(value);
	}

	template<typename _Ret> inline _Ret With(const std::function<_Ret(T &)> &fn) {
		std::lock_guard<mutex_type> lock(mutex);
		return fn(value);
	}
};

#endif
