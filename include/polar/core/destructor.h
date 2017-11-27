#pragma once

#include <functional>

namespace polar::core {
	class destructor {
	  private:
		std::function<void()> fn;

	  public:
		destructor(std::function<void()> fn) : fn(fn) {}
		~destructor() { fn(); }
	};
} // namespace polar::core
