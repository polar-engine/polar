#pragma once

#include <polar/core/ecs.h>
#include <polar/core/types.h>
#include <polar/property/base.h>
#include <vector>

namespace polar::component {
	class base : public core::ecs<property::base> {
	  public:
		template<typename T> using basic_getter_type = std::function<Decimal(T *)>;
		template<typename T> using basic_setter_type = std::function<void(T *, Decimal)>;
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

		virtual std::string name() const = 0;

		virtual accessor_list accessors() const {
			accessor_list l;
			return l;
		}
	};
}
