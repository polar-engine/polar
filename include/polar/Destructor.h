#pragma once

#include <functional>

class Destructor {
private:
	std::function<void()> fn;
public:
	Destructor(std::function<void()> fn) : fn(fn) {}
	~Destructor() { fn(); }
};
