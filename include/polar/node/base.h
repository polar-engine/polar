#pragma once

#include <polar/core/registry.h>
#include <polar/core/serializer.h>
#include <variant>

#define NODE_BEGIN(N, T)                                                        \
	struct N : polar::node::base<T> {

#define NODE_END(N, T, NAME)                                                    \
		std::string name() const override { return NAME; }                      \
	  private:                                                                  \
		static bool registered;                                                 \
	};                                                                          \
	bool N::registered = polar::core::registry::node<T>::reg(NAME, [](auto s) { \
		return N::deserialize(s);                                               \
	});

namespace polar::core {
	class polar;
} // namespace polar::core

namespace polar::node {
	template<typename T>
	struct base {
		virtual ~base() {}

		virtual std::string name() const = 0;
		virtual core::store_serializer & serialize(core::store_serializer &) const = 0;
		virtual T eval(core::polar *) = 0;
	};
} // namespace polar::node

namespace polar {
	template<typename T>
	class wrapped_node {
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

		friend inline core::store_serializer & operator<<(core::store_serializer &s, const wrapped_node<T> &n) {
			std::visit([&s](auto &&arg) {
				using VT = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<VT, T>) {
					s << "" << arg;
				} else if constexpr(std::is_same_v<VT, std::shared_ptr<node::base<T>>>) {
					s << arg->name();
					arg->serialize(s);
				}
			}, n.data);
			return s;
		}

		friend inline core::deserializer & operator>>(core::deserializer &s, wrapped_node<T> &n) {
			n = core::registry::node<T>::deserialize(s);
			return s;
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
