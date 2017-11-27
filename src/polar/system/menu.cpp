#include <polar/asset/audio.h>
#include <polar/component/audiosource.h>
#include <polar/component/color.h>
#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/sprite/box.h>
#include <polar/component/sprite/slider.h>
#include <polar/component/text.h>
#include <polar/support/audio/sourcetype.h>
#include <polar/support/input/key.h>
#include <polar/support/ui/control.h>
#include <polar/system/asset.h>
#include <polar/system/input.h>
#include <polar/system/menu.h>
#include <polar/system/tweener.h>

namespace polar::system {
	void menu::activate() {
		auto m     = getcurrentmenu();
		auto &item = m->at(current);

		if(item.control) {
			if(item.control->activate()) {
				render(current, true);
				if(item.fn(item.control->get())) {
					auto assetM = engine->get<asset>().lock();
					IDType soundID;
					soundDtors[soundIndex++] = engine->add(&soundID);
					soundIndex %= soundDtors.size();
					engine->add<component::audiosource>(
					    soundID, assetM->get<polar::asset::audio>("menu1"),
					    support::audio::sourcetype::effect);
				}
			}
		} else if(!item.children.empty()) {
			navigate(0, 1, true);
		}
	}

	void menu::navigate(int down, int right, bool force) {
		auto m     = getcurrentmenu();
		auto &item = m->at(current);

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
				auto m  = getcurrentmenu();
				auto &i = m->at(current);
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
					current = stack.back();
					stack.pop_back();
					++right;
					playBeep = true;
				}
			}
			render_all();
		}
		if(down != 0) {
			auto m        = getcurrentmenu();
			auto previous = current;
			current += down;
			if(current < 0) {
				current += m->size();
			} else {
				current %= m->size();
			}
			render(previous, true);
			playBeep = true;
		}

		if(playBeep) {
			auto assetM = engine->get<asset>().lock();
			IDType soundID;
			soundDtors[soundIndex++] = engine->add(&soundID);
			soundIndex %= soundDtors.size();
			engine->add<component::audiosource>(
			    soundID, assetM->get<polar::asset::audio>("menu1"),
			    support::audio::sourcetype::effect);
		}
	}

	void menu::render(size_t i, bool replace) {
		auto m     = getcurrentmenu();
		auto &item = m->at(i);

		IDType id;
		if(replace) {
			itemDtors.at(i) = engine->add(&id);
		} else {
			itemDtors.emplace_back(engine->add(&id));
		}

		const Decimal uiHeight     = 2.25;
		const Decimal uiTextHeight = 160;
		const Decimal uiTextWidth  = 550;

		/* max 6 items on screen at max scale of 0.375
		* 6 * 0.375 = 2.25 numerator
		*/
		Decimal scale =
		    glm::min(Decimal(uiHeight) / Decimal(m->size()), Decimal(uiScale));
		Decimal spacing = uiTextHeight * scale;
		Point2 origin   = Point2(60, 50 + spacing * (m->size() - i - 1));

		engine->add<component::text>(id, font, item.value);
		engine->add<component::screenposition>(id, origin);
		engine->add<component::scale>(id, Point3(scale));

		if(int(i) == current) {
			engine->add<component::color>(id, Point4(1, 1, selectionAlpha, 1));
		}

		if(item.control) {
			IDType controlID;
			auto offset = Point2(uiTextWidth / uiScale * scale, 0);
			offset.y -= 12 * scale;
			controlDtors[i] = item.control->render(engine, controlID,
			                                       origin + offset, 8 * scale);
		}
	}

	void menu::init() {
		using key_t = support::input::key;

		auto assetM = engine->get<asset>().lock();
		auto inputM = engine->get<input>().lock();
		auto tw     = engine->get<tweener<float>>().lock();

		assetM->request<polar::asset::audio>("menu1");

		font = assetM->get<polar::asset::font>("nasalization-rg");

		for(auto k : {key_t::Down, key_t::S}) {
			dtors.emplace_back(inputM->on(k, [this](key_t) { navigate(1); }));
		}

		for(auto k : {key_t::Up, key_t::W}) {
			dtors.emplace_back(inputM->on(k, [this](key_t) { navigate(-1); }));
		}

		for(auto k : {key_t::Left, key_t::A}) {
			dtors.emplace_back(
			    inputM->on(k, [this](key_t) { navigate(0, -1); }));
		}

		for(auto k : {key_t::Right, key_t::D}) {
			dtors.emplace_back(
			    inputM->on(k, [this](key_t) { navigate(0, 1); }));
		}

		for(auto k : {key_t::Space, key_t::Enter, key_t::MouseLeft,
		              key_t::ControllerA}) {
			dtors.emplace_back(inputM->on(k, [this](key_t) { activate(); }));
		}

		for(auto k : {key_t::Escape, key_t::Backspace, key_t::MouseRight,
		              key_t::ControllerBack}) {
			dtors.emplace_back(
			    inputM->on(k, [this](key_t) { navigate(0, -1, true); }));
		}

		dtors.emplace_back(inputM->onmousewheel([this](const Point2 &delta) {
			navigate(int(-delta.y), int(delta.x));
		}));

		dtors.emplace_back(
		    inputM->ondigital("menu_up", [this]() { navigate(-1); }));
		dtors.emplace_back(
		    inputM->ondigital("menu_down", [this]() { navigate(1); }));
		dtors.emplace_back(
		    inputM->ondigital("menu_left", [this]() { navigate(0, -1); }));
		dtors.emplace_back(
		    inputM->ondigital("menu_right", [this]() { navigate(0, 1); }));
		dtors.emplace_back(
		    inputM->ondigital("menu_confirm", [this]() { activate(); }));
		dtors.emplace_back(inputM->ondigital(
		    "menu_back", [this]() { navigate(0, -1, true); }));

		dtors.emplace_back(
		    tw->tween(0.0f, 1.0f, 0.25, true,
		              [this](core::polar *, const float &x) {
			              selectionAlpha = x;
			          }));

		render_all();
	}
}
