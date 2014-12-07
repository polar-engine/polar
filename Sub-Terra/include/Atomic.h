#pragma once

template<typename T, typename Mutex = std::recursive_mutex> class Atomic {
private:
	T value;
	Mutex mutex;
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
