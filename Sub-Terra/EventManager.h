#pragma once

#include "System.h"

class EventManager : public System {
private:
	template<typename ...Ts> std::vector<const std::function<void(const std::string &, const std::function<void(Ts...)>)>> _globalListeners;
	template<typename ...Ts> std::unordered_multimap<std::string, const std::function<void(Ts...)>> _listeners;
public:
	template<typename ...Ts> void Listen(const std::function<void(const std::string &, Ts...)> &fn) {
		_globalListeners.emplace(Tag<Ts...>, fn);
	}
	template<typename ...Ts> void ListenFor(const std::string &msg, const std::function<void(Ts...)> &fn) {
		ListenFor("", msg, fn);
	}
	template<typename ...Ts> void ListenFor(const std::string &ns, const std::string &msg, const std::function<void(Ts...)> &fn) {
		_listeners.emplace(ns + '.' + msg, fn);
	}
	template<typename ...Ts> void Fire(const std::string &msg, Ts ...args) const {
		FireIn("", msg, args...);
	}
	template<typename ...Ts> void FireIn(const std::string &ns, const std::string &msg, Ts ...) const {
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
