#pragma once

#include <functional>
#include <optional>
#include <polar/core/ecs.h>
#include <polar/math/types.h>
#include <polar/node/base.h>
#include <polar/property/base.h>
#include <string>
#include <vector>

namespace polar::component {
	class base : public core::ecs<property::base> {
	  public:
		template<typename T> using basic_getter_type = std::function<math::decimal(T *)>;
		template<typename T> using basic_setter_type = std::function<void(T *, math::decimal)>;
		using getter_type = basic_getter_type<base>;
		using setter_type = basic_setter_type<base>;

		struct accessor_type {
			std::optional<getter_type> getter;
			std::optional<setter_type> setter;
		};

		template<typename T>
		static accessor_type make_accessor(basic_getter_type<T> getter, basic_setter_type<T> setter) {
			return accessor_type{
				*reinterpret_cast<getter_type *>(&getter),
				*reinterpret_cast<setter_type *>(&setter)
			};
		}

		using accessor_pair = std::pair<std::string, accessor_type>;
		using accessor_list = std::vector<accessor_pair>;

		virtual ~base() {}

		virtual std::string name() const = 0;

		virtual accessor_list accessors() const {
			accessor_list l;
			return l;
		}
	};
} // namespace polar::component
