#pragma once

#include "System.h"

typedef std::function<void(const std::string &, Arg)> GlobalListener;
typedef std::function<void(Arg)> Listener;

class EventManager : public System {
private:
	std::vector<const GlobalListener> _globalListeners;
	std::unordered_multimap<std::string, const Listener> _listeners;
public:
	static bool IsSupported() { return true; }
	EventManager(const Polar *engine) : System(engine) {}
	void Listen(const GlobalListener &fn) {
		_globalListeners.emplace_back(fn);
	}
	void ListenFor(const std::string &msg, const Listener &fn) {
		ListenFor("", msg, fn);
	}
	void ListenFor(const std::string &ns, const std::string &msg, const Listener &fn) {
		_listeners.emplace(ns + '.' + msg, fn);
	}
	void Fire(const std::string &msg, Arg arg = nullptr) const {
		FireIn("", msg, arg);
	}
	void FireIn(const std::string &ns, const std::string &msg, Arg arg = nullptr) const {
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
