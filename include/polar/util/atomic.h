#pragma once

/*#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
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
#include <polar/core/debugmanager.h>

template<typename T> class atomic {
public:
	typedef std::recursive_mutex mutex_type;
private:
	T value;
	mutex_type mutex;
	std::condition_variable cv;
public:
	template<typename ...Ts> atomic(Ts && ...args) : value(std::forward<Ts>(args)...) {}

	inline void notify() {
		cv.notify_one();
	}

	inline void wait(const std::function<bool(T &)> &pred, const std::function<void(T &)> &fn) {
		polar::debugmanager()->fatal("atomic::wait: not implemented");
		/*std::unique_lock<std::mutex> lock(mutex);
		while(!pred(value)) {
			cv.wait(lock);
		}
		fn(value);*/
	}

	inline void with(const std::function<void(T &)> &fn) {
		std::lock_guard<mutex_type> lock(mutex);
		fn(value);
	}

	template<typename _Ret> inline _Ret with(const std::function<_Ret(T &)> &fn) {
		std::lock_guard<mutex_type> lock(mutex);
		return fn(value);
	}
};

//#endif