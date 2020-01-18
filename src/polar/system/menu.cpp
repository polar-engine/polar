#include <polar/asset/audio.h>
#include <polar/component/audiosource.h>
#include <polar/component/color.h>
#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/sprite/box.h>
#include <polar/component/sprite/slider.h>
#include <polar/component/text.h>
#include <polar/support/action/menu.h>
#include <polar/support/action/mouse.h>
#include <polar/support/audio/sourcetype.h>
#include <polar/support/input/key.h>
#include <polar/support/ui/control.h>
#include <polar/system/action.h>
#include <polar/system/asset.h>
#include <polar/system/menu.h>
#include <polar/system/renderer/base.h>
#include <polar/system/tweener.h>

namespace polar::system {
	void menu::activate() {
		auto &item = current_at(current);

		if(item.control) {
			if(item.control->activate()) {
				render(current, true);
				if(item.fn(item.control->get())) {
					auto assetM = engine->get<asset>().lock();
					auto sound = engine->add();
					sounds[soundIndex++] = sound;
					soundIndex %= sounds.size();
					engine->add<component::audiosource>(
					    sound, assetM->get<polar::asset::audio>("menu1"),
					    support::audio::sourcetype::effect);
				}
			}
		} else if(!item.children.empty()) {
			navigate(0, 1, true);
		}
	}

	void menu::navigate(int down, int right, bool force) {
		auto &item = current_at(current);

		bool playBeep = false;

		bool newForce = force;
		if(!force && right && item.control && item.control->navigate(right)) {
			item.fn(item.control->get());
			render(current, true);
			playBeep = true;
		} else {
			newForce = force;
		}

		if(newForce) {
			while(right > 0) {
				auto &i = current_at(current);
				if(!i.children.empty()) {
					stack.emplace_back(current);
					current = 0;
					--right;
					playBeep = true;
				} else {
					activate();
				}
			}
			while(right < 0) {
				if(stack.empty()) {
					if(force) {
						engine->transition = "back";
						return;
					}
				} else {
					current = (int)stack.back();
					stack.pop_back();
					++right;
					playBeep = true;
				}
			}
			render_all();
		}
		if(down != 0) {
			navigate_to(current + down);
		}

		if(playBeep) {
			beep();
		}
	}

	void menu::navigate_to(int to) {
		if(to == current) { return; }
		log()->debug(to, ' ', current);

		auto sz = current_size();
		auto previous = current;
		current = to;
		if(current < 0) {
			current += int(sz);
		} else {
			current %= int(sz);
		}
		render(previous, true);
		beep();
	}

	void menu::beep() {
		auto assetM = engine->get<asset>().lock();
		auto sound = engine->add();
		sounds[soundIndex++] = sound;
		soundIndex %= sounds.size();
		engine->add<component::audiosource>(
		    sound, assetM->get<polar::asset::audio>("menu1"),
		    support::audio::sourcetype::effect);
	}

	void menu::render(size_t i, bool replace) {
		auto size = current_size();
		if(i < 0 || i >= size) { return; }

		auto &item = current_at(i);

		auto item_object = engine->add();
		if(replace) {
			items.at(i) = item_object;
		} else {
			items.emplace_back(item_object);
		}

		Decimal scale = glm::min(actual_height() / Decimal(size), actual_scale());
		Decimal spacing = uiTextHeight * scale;
		Point2 origin   = Point2(60, 50 + spacing * (size - i - 1));

		engine->add<component::text>(item_object, font, item.value);
		engine->add<component::screenposition>(item_object, origin);
		engine->add<component::scale>(item_object, Point3(scale));

		if(int(i) == current) {
			engine->add<component::color>(item_object, Point4(1, 1, selectionAlpha, 1));
		}

		if(item.control) {
			auto control = engine->add();
			auto offset = Point2(uiTextWidth / uiBase * scale, 0);
			offset.y -= 12 * scale;
			item.control->render(engine, control, origin + offset, 8 * scale);
			controls[(int)i] = control;
		}
	}

	void menu::on_cursor(int y) {
		auto size = current_size();

		auto scale = glm::min(actual_height() / Decimal(size), actual_scale());
		auto spacing = uiTextHeight * scale;

		auto height = engine->get<renderer::base>().lock()->getheight();
		auto origin = height - y;

		/* origin = 50 + spacing * (size - i - 1);
		 * spacing * (size - i - 1) = origin - 50
		 * size - i - 1 = (origin - 50) / spacing
		 * i = size - (origin - 50) / spacing - 1
		 */
		auto i = int(glm::floor(size - (origin - 50) / spacing));
		if(i >= 0 && i < int(size)) { navigate_to(i); }
	}

	void menu::init() {
		using lifetime = support::action::lifetime;
		namespace a = support::action::menu;
		namespace mse = support::action::mouse;

		auto assetM = engine->get<asset>().lock();
		auto act    = engine->get<action>().lock();
		auto tw     = engine->get<tweener<float>>().lock();

		assetM->request<polar::asset::audio>("menu1");

		font = assetM->get<polar::asset::font>("nasalization-rg");

		keep(act->bind<a::down   >(lifetime::on, [this](auto) {
			navigate( 1);
		}));
		keep(act->bind<a::up     >(lifetime::on, [this](auto) {
			navigate(-1);
		}));
		keep(act->bind<a::right  >(lifetime::on, [this](auto) {
			navigate(0,  1);
		}));
		keep(act->bind<a::left   >(lifetime::on, [this](auto) {
			navigate(0, -1);
		}));
		keep(act->bind<a::forward>(lifetime::on, [this](auto) {
			activate();
		}));
		keep(act->bind<a::back   >(lifetime::on, [this](auto) {
			navigate(0, -1, true);
		}));

		keep(act->bind<mse::position_y>([this](auto, Decimal y) {
			if(int(y) != 0) { on_cursor(int(y)); }
		}));

		keep(tw->tween(0.0f, 1.0f, 0.25, true, [this](auto, const float &x) {
			selectionAlpha = x;
		}));

		render_all();
	}
} // namespace polar::system
