#pragma once

#include <functional>

namespace polar {
namespace core {
	class destructor {
	  private:
		std::function<void()> fn;

	  public:
		destructor(std::function<void()> fn) : fn(fn) {}
		~destructor() { fn(); }
	};
}
}
