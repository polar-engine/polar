#pragma once

#include "System.h"

class EventManager : public System {
private:
	std::vector<const std::function<void(const std::string &)>> _globalListeners;
	std::unordered_multimap<std::string, const std::function<void()>> _listeners;
public:
	void Listen(const std::function<void(const std::string &)> &);
	void ListenFor(const std::string &, const std::function<void()> &);
	void Fire(const std::string &);
};
