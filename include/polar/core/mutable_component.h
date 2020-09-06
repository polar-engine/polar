#pragma once

#include <polar/core/destructor.h>

namespace polar::core {
	template<typename T, typename = typename std::enable_if<std::is_base_of<component::base, T>::value>::type>
	class mutable_component {
	  public:
		using dtor_type = std::shared_ptr<destructor>;

	  private:
		std::shared_ptr<T> ptr;
		dtor_type _dtor;

	  public:
		mutable_component(std::shared_ptr<T> ptr, std::function<void()> fn) : mutable_component(ptr, std::make_shared<destructor>(fn)) {}
		mutable_component(std::shared_ptr<T> ptr, dtor_type dtor) : ptr(ptr), _dtor(dtor) {}

		auto dtor() const {
			return _dtor;
		}

		inline operator bool() const {
			return (bool)ptr;
		}

		std::shared_ptr<T> operator->() const {
			return ptr;
		}

		friend inline bool operator==(const mutable_component<T> &lhs, const mutable_component<T> &rhs) {
			return lhs.ptr == rhs.ptr;
		}
	};
} // namespace polar::core
