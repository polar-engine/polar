#pragma once

namespace polar::system {
	class console : public base {
	  protected:
		void init() override {
			namespace kb    = polar::support::action::keyboard;
			using key      = support::input::key;
			using lifetime = support::action::lifetime;

			auto act = engine->get<action>().lock();
			keep(act->bind<kb::key<key::Tilde>>(lifetime::on, [this](auto) {
				engine->transition = "back";
			}, -1, false));

			keep(act->bind<kb::key<key::P>>(lifetime::on, [this](auto) {
				debugmanager()->verbose("P");
			}, -1, false));
		}
	  public:
		static bool supported() { return true; }

		console(core::polar *engine) : base(engine) {}
	};
} // namespace polar::system
