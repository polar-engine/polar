#pragma once

#include <polar/asset/font.h>
#include <polar/component/color.h>
#include <polar/component/screenposition.h>
#include <polar/component/text.h>

namespace polar::system {
	class console : public base {
	  private:
		using key      = support::input::key;
		using lifetime = support::action::lifetime;

		std::ostringstream buffer;
		core::ref bufferDtor;
		IDType bufferID;

		void process(key k) {
			switch(k) {
			case key::A:     buffer << 'a'; break;
			case key::B:     buffer << 'b'; break;
			case key::C:     buffer << 'c'; break;
			case key::D:     buffer << 'd'; break;
			case key::E:     buffer << 'e'; break;
			case key::F:     buffer << 'f'; break;
			case key::G:     buffer << 'g'; break;
			case key::H:     buffer << 'h'; break;
			case key::I:     buffer << 'i'; break;
			case key::J:     buffer << 'j'; break;
			case key::K:     buffer << 'k'; break;
			case key::L:     buffer << 'l'; break;
			case key::M:     buffer << 'm'; break;
			case key::N:     buffer << 'n'; break;
			case key::O:     buffer << 'o'; break;
			case key::P:     buffer << 'p'; break;
			case key::Q:     buffer << 'q'; break;
			case key::R:     buffer << 'r'; break;
			case key::S:     buffer << 's'; break;
			case key::T:     buffer << 't'; break;
			case key::U:     buffer << 'u'; break;
			case key::V:     buffer << 'v'; break;
			case key::W:     buffer << 'w'; break;
			case key::X:     buffer << 'x'; break;
			case key::Y:     buffer << 'y'; break;
			case key::Z:     buffer << 'z'; break;
			case key::Space: buffer << ' '; break;
			case key::Enter:
				engine->transition = "back";
				break;
			}
			render();
		}

		void render() {
			bufferDtor = engine->add(bufferID);

			auto assetM = engine->get<asset>().lock();
			auto font   = assetM->get<polar::asset::font>("nasalization-rg");

			engine->add<component::text>(bufferID, font, buffer.str());
			engine->add<component::screenposition>(bufferID, Point2(5, 5), support::ui::origin::topleft);
			engine->add<component::color>(bufferID, Point4(1, 1, 1, 0.8));
			engine->add<component::scale>(bufferID, Point3(0.1));
		}
	  protected:
		void init() override {
			namespace kb = polar::support::action::keyboard;

			auto act = engine->get<action>().lock();
			keep(act->bind<kb::key<key::Tilde>>(lifetime::on, [this](auto) {
				engine->transition = "back";
			}, -1, false));

			keep(act->bind<kb::key<key::A    >>(lifetime::on, [this](auto) { process(key::A);     }, -100, false));
			keep(act->bind<kb::key<key::B    >>(lifetime::on, [this](auto) { process(key::B);     }, -100, false));
			keep(act->bind<kb::key<key::C    >>(lifetime::on, [this](auto) { process(key::C);     }, -100, false));
			keep(act->bind<kb::key<key::D    >>(lifetime::on, [this](auto) { process(key::D);     }, -100, false));
			keep(act->bind<kb::key<key::E    >>(lifetime::on, [this](auto) { process(key::E);     }, -100, false));
			keep(act->bind<kb::key<key::F    >>(lifetime::on, [this](auto) { process(key::F);     }, -100, false));
			keep(act->bind<kb::key<key::G    >>(lifetime::on, [this](auto) { process(key::G);     }, -100, false));
			keep(act->bind<kb::key<key::H    >>(lifetime::on, [this](auto) { process(key::H);     }, -100, false));
			keep(act->bind<kb::key<key::I    >>(lifetime::on, [this](auto) { process(key::I);     }, -100, false));
			keep(act->bind<kb::key<key::J    >>(lifetime::on, [this](auto) { process(key::J);     }, -100, false));
			keep(act->bind<kb::key<key::K    >>(lifetime::on, [this](auto) { process(key::K);     }, -100, false));
			keep(act->bind<kb::key<key::L    >>(lifetime::on, [this](auto) { process(key::L);     }, -100, false));
			keep(act->bind<kb::key<key::M    >>(lifetime::on, [this](auto) { process(key::M);     }, -100, false));
			keep(act->bind<kb::key<key::N    >>(lifetime::on, [this](auto) { process(key::N);     }, -100, false));
			keep(act->bind<kb::key<key::O    >>(lifetime::on, [this](auto) { process(key::O);     }, -100, false));
			keep(act->bind<kb::key<key::P    >>(lifetime::on, [this](auto) { process(key::P);     }, -100, false));
			keep(act->bind<kb::key<key::Q    >>(lifetime::on, [this](auto) { process(key::Q);     }, -100, false));
			keep(act->bind<kb::key<key::R    >>(lifetime::on, [this](auto) { process(key::R);     }, -100, false));
			keep(act->bind<kb::key<key::S    >>(lifetime::on, [this](auto) { process(key::S);     }, -100, false));
			keep(act->bind<kb::key<key::T    >>(lifetime::on, [this](auto) { process(key::T);     }, -100, false));
			keep(act->bind<kb::key<key::U    >>(lifetime::on, [this](auto) { process(key::U);     }, -100, false));
			keep(act->bind<kb::key<key::V    >>(lifetime::on, [this](auto) { process(key::V);     }, -100, false));
			keep(act->bind<kb::key<key::W    >>(lifetime::on, [this](auto) { process(key::W);     }, -100, false));
			keep(act->bind<kb::key<key::X    >>(lifetime::on, [this](auto) { process(key::X);     }, -100, false));
			keep(act->bind<kb::key<key::Y    >>(lifetime::on, [this](auto) { process(key::Y);     }, -100, false));
			keep(act->bind<kb::key<key::Z    >>(lifetime::on, [this](auto) { process(key::Z);     }, -100, false));
			keep(act->bind<kb::key<key::Space>>(lifetime::on, [this](auto) { process(key::Space); }, -100, false));
			keep(act->bind<kb::key<key::Enter>>(lifetime::on, [this](auto) { process(key::Enter); }, -100, false));

			keep(act->bind<kb::key<key::A    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::B    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::C    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::D    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::E    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::F    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::G    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::H    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::I    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::J    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::K    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::L    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::M    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::N    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::O    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::P    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Q    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::R    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::S    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::T    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::U    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::V    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::W    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::X    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Y    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Z    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Space>>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Enter>>(lifetime::when, [this](auto) {}, -100, false));
		}
	  public:
		static bool supported() { return true; }

		console(core::polar *engine) : base(engine) {}
	};
} // namespace polar::system
