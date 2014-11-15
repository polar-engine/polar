#include "stdafx.h"
#include "EventManager.h"

void EventManager::Listen(const std::function<void(const std::string &)> &fn) {
	_globalListeners.push_back(fn);
}

void EventManager::ListenFor(const std::string &msg, const std::function<void()> &fn) {
	_listeners.emplace(msg, fn);
}

void EventManager::Fire(const std::string &msg) {
	auto range = _listeners.equal_range(msg);
	for(auto i = range.first; i != range.second; ++i) {
		i->second();
	}
	for(auto &listener : _globalListeners) {
		listener(msg);
	}
}
