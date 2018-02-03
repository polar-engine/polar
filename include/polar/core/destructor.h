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

	class ref {
	private:
		std::shared_ptr<destructor> dtor;
	public:
		ref(std::function<void()> fn = [] {}) : dtor(std::make_shared<destructor>(fn)) {}
	};
} // namespace polar::core
