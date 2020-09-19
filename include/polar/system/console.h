#pragma once

#include <polar/api/kludge.h>
#include <polar/asset/font.h>
#include <polar/component/color.h>
#include <polar/component/screenposition.h>
#include <polar/component/text.h>

namespace polar::system {
	class console : public base {
	  private:
		using key      = support::input::key;
		using lifetime = support::action::lifetime;

		std::string buffer;
		core::ref buffer_object;

		void handle(key k) {
			switch(k) {
			case key::A:      buffer.push_back('a'); break;
			case key::B:      buffer.push_back('b'); break;
			case key::C:      buffer.push_back('c'); break;
			case key::D:      buffer.push_back('d'); break;
			case key::E:      buffer.push_back('e'); break;
			case key::F:      buffer.push_back('f'); break;
			case key::G:      buffer.push_back('g'); break;
			case key::H:      buffer.push_back('h'); break;
			case key::I:      buffer.push_back('i'); break;
			case key::J:      buffer.push_back('j'); break;
			case key::K:      buffer.push_back('k'); break;
			case key::L:      buffer.push_back('l'); break;
			case key::M:      buffer.push_back('m'); break;
			case key::N:      buffer.push_back('n'); break;
			case key::O:      buffer.push_back('o'); break;
			case key::P:      buffer.push_back('p'); break;
			case key::Q:      buffer.push_back('q'); break;
			case key::R:      buffer.push_back('r'); break;
			case key::S:      buffer.push_back('s'); break;
			case key::T:      buffer.push_back('t'); break;
			case key::U:      buffer.push_back('u'); break;
			case key::V:      buffer.push_back('v'); break;
			case key::W:      buffer.push_back('w'); break;
			case key::X:      buffer.push_back('x'); break;
			case key::Y:      buffer.push_back('y'); break;
			case key::Z:      buffer.push_back('z'); break;
			case key::Num1:   buffer.push_back('1'); break;
			case key::Num2:   buffer.push_back('2'); break;
			case key::Num3:   buffer.push_back('3'); break;
			case key::Num4:   buffer.push_back('4'); break;
			case key::Num5:   buffer.push_back('5'); break;
			case key::Num6:   buffer.push_back('6'); break;
			case key::Num7:   buffer.push_back('7'); break;
			case key::Num8:   buffer.push_back('8'); break;
			case key::Num9:   buffer.push_back('9'); break;
			case key::Num0:   buffer.push_back('0'); break;
			case key::Hyphen: buffer.push_back('-'); break;
			case key::Equals: buffer.push_back('='); break;
			case key::Comma:  buffer.push_back(','); break;
			case key::Period: buffer.push_back('.'); break;
			case key::Space:  buffer.push_back(' '); break;
			case key::Enter:
				process(buffer);
				engine.transition = "back";
				break;
			case key::Backspace:
				if(!buffer.empty()) { buffer.pop_back(); }
				break;
			default:
				break;
			}
			render();
		}

		void process(std::string line) {
			polar::api::kludge kl{engine};

			auto t = kl.lex(line);
			log()->verbose("console", t);

			auto e = kl.parse(t);
			log()->verbose("console", e);

			kl.reduce(e);
			log()->verbose("console", e);

			auto r = kl.exec(e);
			if(std::holds_alternative<polar::api::kludge::exec_error>(r)) {
				log()->critical("console", "kludge exec failed");
			}
		}

		void render() {
			buffer_object = engine.add();

			/* XXX
			auto assetM = engine.get<asset>().lock();
			auto font   = assetM->get<polar::asset::font>("nasalization-rg");

			engine.add<component::text>(buffer_object, font, buffer);
			*/
			engine.add<component::screenposition>(buffer_object, math::point2(5, 5), support::ui::origin::topleft);
			engine.add<component::color>(buffer_object, math::point4(1, 1, 1, 0.8));
			engine.add<component::scale>(buffer_object, math::point3(0.1f));
		}
	  protected:
		void init() override {
			namespace kb = polar::support::action::keyboard;

			auto act = engine.get<action>().lock();
			keep(act->bind<kb::key<key::Tilde>>(lifetime::on, [this](auto) {
				engine.transition = "back";
			}, -1, false));

			keep(act->bind<kb::key<key::A        >>(lifetime::on, [this](auto) { handle(key::A);         }, -100, false));
			keep(act->bind<kb::key<key::B        >>(lifetime::on, [this](auto) { handle(key::B);         }, -100, false));
			keep(act->bind<kb::key<key::C        >>(lifetime::on, [this](auto) { handle(key::C);         }, -100, false));
			keep(act->bind<kb::key<key::D        >>(lifetime::on, [this](auto) { handle(key::D);         }, -100, false));
			keep(act->bind<kb::key<key::E        >>(lifetime::on, [this](auto) { handle(key::E);         }, -100, false));
			keep(act->bind<kb::key<key::F        >>(lifetime::on, [this](auto) { handle(key::F);         }, -100, false));
			keep(act->bind<kb::key<key::G        >>(lifetime::on, [this](auto) { handle(key::G);         }, -100, false));
			keep(act->bind<kb::key<key::H        >>(lifetime::on, [this](auto) { handle(key::H);         }, -100, false));
			keep(act->bind<kb::key<key::I        >>(lifetime::on, [this](auto) { handle(key::I);         }, -100, false));
			keep(act->bind<kb::key<key::J        >>(lifetime::on, [this](auto) { handle(key::J);         }, -100, false));
			keep(act->bind<kb::key<key::K        >>(lifetime::on, [this](auto) { handle(key::K);         }, -100, false));
			keep(act->bind<kb::key<key::L        >>(lifetime::on, [this](auto) { handle(key::L);         }, -100, false));
			keep(act->bind<kb::key<key::M        >>(lifetime::on, [this](auto) { handle(key::M);         }, -100, false));
			keep(act->bind<kb::key<key::N        >>(lifetime::on, [this](auto) { handle(key::N);         }, -100, false));
			keep(act->bind<kb::key<key::O        >>(lifetime::on, [this](auto) { handle(key::O);         }, -100, false));
			keep(act->bind<kb::key<key::P        >>(lifetime::on, [this](auto) { handle(key::P);         }, -100, false));
			keep(act->bind<kb::key<key::Q        >>(lifetime::on, [this](auto) { handle(key::Q);         }, -100, false));
			keep(act->bind<kb::key<key::R        >>(lifetime::on, [this](auto) { handle(key::R);         }, -100, false));
			keep(act->bind<kb::key<key::S        >>(lifetime::on, [this](auto) { handle(key::S);         }, -100, false));
			keep(act->bind<kb::key<key::T        >>(lifetime::on, [this](auto) { handle(key::T);         }, -100, false));
			keep(act->bind<kb::key<key::U        >>(lifetime::on, [this](auto) { handle(key::U);         }, -100, false));
			keep(act->bind<kb::key<key::V        >>(lifetime::on, [this](auto) { handle(key::V);         }, -100, false));
			keep(act->bind<kb::key<key::W        >>(lifetime::on, [this](auto) { handle(key::W);         }, -100, false));
			keep(act->bind<kb::key<key::X        >>(lifetime::on, [this](auto) { handle(key::X);         }, -100, false));
			keep(act->bind<kb::key<key::Y        >>(lifetime::on, [this](auto) { handle(key::Y);         }, -100, false));
			keep(act->bind<kb::key<key::Z        >>(lifetime::on, [this](auto) { handle(key::Z);         }, -100, false));
			keep(act->bind<kb::key<key::Num1     >>(lifetime::on, [this](auto) { handle(key::Num1);      }, -100, false));
			keep(act->bind<kb::key<key::Num2     >>(lifetime::on, [this](auto) { handle(key::Num2);      }, -100, false));
			keep(act->bind<kb::key<key::Num3     >>(lifetime::on, [this](auto) { handle(key::Num3);      }, -100, false));
			keep(act->bind<kb::key<key::Num4     >>(lifetime::on, [this](auto) { handle(key::Num4);      }, -100, false));
			keep(act->bind<kb::key<key::Num5     >>(lifetime::on, [this](auto) { handle(key::Num5);      }, -100, false));
			keep(act->bind<kb::key<key::Num6     >>(lifetime::on, [this](auto) { handle(key::Num6);      }, -100, false));
			keep(act->bind<kb::key<key::Num7     >>(lifetime::on, [this](auto) { handle(key::Num7);      }, -100, false));
			keep(act->bind<kb::key<key::Num8     >>(lifetime::on, [this](auto) { handle(key::Num8);      }, -100, false));
			keep(act->bind<kb::key<key::Num9     >>(lifetime::on, [this](auto) { handle(key::Num9);      }, -100, false));
			keep(act->bind<kb::key<key::Num0     >>(lifetime::on, [this](auto) { handle(key::Num0);      }, -100, false));
			keep(act->bind<kb::key<key::Hyphen   >>(lifetime::on, [this](auto) { handle(key::Hyphen);    }, -100, false));
			keep(act->bind<kb::key<key::Equals   >>(lifetime::on, [this](auto) { handle(key::Equals);    }, -100, false));
			keep(act->bind<kb::key<key::Comma    >>(lifetime::on, [this](auto) { handle(key::Comma);     }, -100, false));
			keep(act->bind<kb::key<key::Period   >>(lifetime::on, [this](auto) { handle(key::Period);    }, -100, false));
			keep(act->bind<kb::key<key::Space    >>(lifetime::on, [this](auto) { handle(key::Space);     }, -100, false));
			keep(act->bind<kb::key<key::Enter    >>(lifetime::on, [this](auto) { handle(key::Enter);     }, -100, false));
			keep(act->bind<kb::key<key::Backspace>>(lifetime::on, [this](auto) { handle(key::Backspace); }, -100, false));

			keep(act->bind<kb::key<key::A        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::B        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::C        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::D        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::E        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::F        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::G        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::H        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::I        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::J        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::K        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::L        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::M        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::N        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::O        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::P        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Q        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::R        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::S        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::T        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::U        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::V        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::W        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::X        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Y        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Z        >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num1     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num2     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num3     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num4     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num5     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num6     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num7     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num8     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num9     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Num0     >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Hyphen   >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Equals   >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Comma    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Period   >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Space    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Enter    >>(lifetime::when, [this](auto) {}, -100, false));
			keep(act->bind<kb::key<key::Backspace>>(lifetime::when, [this](auto) {}, -100, false));
		}
	  public:
		static bool supported() { return true; }

		console(core::polar &engine) : base(engine) {}

		virtual std::string name() const override { return "console"; }
	};
} // namespace polar::system
