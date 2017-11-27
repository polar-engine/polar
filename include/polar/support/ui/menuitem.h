#pragma once

namespace polar::support::ui {
	class menuitem {
	  public:
		using FTy = bool(float);

		std::string value;
		std::vector<menuitem> children = {};
		std::function<FTy> fn;
		std::shared_ptr<control::base> control;

		menuitem(std::string value, std::vector<menuitem> children)
		    : value(value), children(children) {}
		menuitem(std::string value, std::function<FTy> fn)
		    : menuitem(value, control::button(), fn) {}
		template<typename T>
		menuitem(std::string value, T control, std::function<FTy> fn)
		    : value(value), fn(fn) {
			static_assert(std::is_base_of<control::base, T>::value,
			              "menuitem requires object of base class "
			              "polar::support::ui::control::base");
			this->control = std::shared_ptr<control::base>(new T(control));
		}
	};
} // namespace polar::support::ui
