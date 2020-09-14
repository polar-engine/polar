#pragma once

#include <variant>

namespace polar::core {
	class polar;
} // namespace polar::core

namespace polar::node {
	template<typename T>
	struct base {
		virtual ~base() {}

		virtual T eval(core::polar *) = 0;
	};
} // namespace polar::node

namespace polar {
	template<typename T>
	struct wrapped_node {
	  private:
		std::variant<
			T,
			std::shared_ptr<node::base<T>>
		> data;

	  public:
		wrapped_node(const T &x) : data(x) {}

		template<
			typename N,
			typename = typename std::enable_if<std::is_base_of<node::base<T>, N>::value>::type
		>
		wrapped_node(const N &n) : data(std::static_pointer_cast<node::base<T>>(std::make_shared<N>(n))) {}

		inline wrapped_node<T> & operator=(const T &x) {
			data = x;
			return *this;
		}

		T eval(core::polar *engine) const {
			T r = std::visit([engine](auto &&arg) -> T {
				using VT = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<VT, T>) {
					return arg;
				} else if constexpr(std::is_same_v<VT, std::shared_ptr<node::base<T>>>) {
					return arg->eval(engine);
				}
			}, data);
			return r;
		}
	};
} // namespace polar
