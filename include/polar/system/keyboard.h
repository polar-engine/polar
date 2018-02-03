#pragma once

#include <array>
#include <polar/system/action.h>
#include <polar/support/input/key.h>

namespace polar::system {
	class keyboard : public system::base {
	private:
		using key_t = support::input::key;

		std::vector<action::digital_ref> actions;
	public:
		static bool supported() { return true; }
		keyboard(core::polar *engine) : base(engine) {}

		void init() final {
			auto act = engine->get<system::action>().lock();

			actions.reserve(size_t(key_t::SIZE));
			for(size_t k = 0; k < size_t(key_t::SIZE); ++k) {
				actions.emplace_back(act->digital());
			}
		}

		auto action(key_t k) {
			return actions.at(size_t(k));
		}
	};
}
