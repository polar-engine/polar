#pragma once

#include "System.h"

union Arg {
	float float_;
	void *pVoid;

	Arg(float f) { float_ = f; }
	Arg(std::nullptr_t) { pVoid = nullptr; }
	template<typename T> Arg(T *p) { pVoid = reinterpret_cast<void *>(p); }
	template<typename T> inline T * Get() { return reinterpret_cast<T *>(pVoid); }
};

typedef std::function<void(const std::string &, Arg)> GlobalListener;
typedef std::function<void(Arg)> Listener;

class EventManager : public System {
private:
	std::vector<GlobalListener> _globalListeners;
	std::unordered_multimap<std::string, Listener> _listeners;
public:
	static bool IsSupported() { return true; }
	EventManager(Polar *engine) : System(engine) {}
	inline void Listen(const GlobalListener &fn) {
		_globalListeners.emplace_back(fn);
	}
	inline void ListenFor(const std::string &msg, const Listener &fn) {
		ListenFor("", msg, fn);
	}
	inline void ListenFor(const std::string &ns, const std::string &msg, const Listener &fn) {
		_listeners.emplace(ns + '.' + msg, fn);
	}
	inline void Fire(const std::string &msg, Arg arg = nullptr) const {
		FireIn("", msg, arg);
	}
	inline void FireIn(const std::string &ns, const std::string &msg, Arg arg = nullptr) const {
		auto m = ns + '.' + msg;
		auto range = _listeners.equal_range(m);
		for(auto i = range.first; i != range.second; ++i) {
			i->second(arg);
		}
		for(auto &listener : _globalListeners) {
			listener(m, arg);
		}
	}
};
