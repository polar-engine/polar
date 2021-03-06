#if !defined(POLAR_H)
#include <polar/core/polar.h>
#else
#pragma once

#include <memory>
#include <polar/component/base.h>
#include <polar/core/deltaticks.h>
#include <polar/core/ref.h>
#include <vector>

namespace polar::system {
	class base {
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
	  private:
		std::vector<core::ref> dtors;
	  protected:
		core::polar *engine;

		inline void keep(core::ref r) {
			dtors.emplace_back(r);
		}

	  public:
		static bool supported() { return false; }

		base(core::polar *engine) : engine(engine) {}
		virtual ~base() {}

		virtual std::string name() const = 0;

		virtual accessor_list accessors() const {
			accessor_list l;
			return l;
		}

		virtual void init() {}
		virtual void update(DeltaTicks &) {}
		virtual void system_added(std::type_index, std::weak_ptr<system::base>) {}
		virtual void component_added(core::weak_ref, std::type_index, std::weak_ptr<component::base>) {}
		virtual void component_removed(core::weak_ref, std::type_index) {}
	};
} // namespace polar::system

#endif
