#pragma once

#include <polar/component/color.h>
#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/sprite/box.h>
#include <polar/component/sprite/slider.h>
#include <polar/component/text.h>
#include <polar/support/input/key.h>
#include <polar/support/ui/control.h>
#include <polar/support/ui/menuitem.h>
#include <polar/system/base.h>
#include <polar/system/tweener.h>
#include <stdint.h>
#include <vector>

namespace polar {
namespace system {
	using menuitem_vector_t = std::vector<support::ui::menuitem>;

	class menu : public base {
		using key_t = support::input::key;

	  private:
		menuitem_vector_t _menu;
		std::vector<size_t> stack;
		int current = 0;

		std::shared_ptr<polar::asset::font> font;
		std::vector<std::shared_ptr<core::destructor>> itemDtors;
		std::unordered_map<int, std::shared_ptr<core::destructor>> controlDtors;
		float selectionAlpha = 0.0f;

		// the size of the array determines how many concurrent sounds we can
		// play at once
		std::array<std::shared_ptr<core::destructor>, 4> soundDtors;
		size_t soundIndex = 0;

		menuitem_vector_t *getcurrentmenu() {
			menuitem_vector_t *m = &_menu;
			for(auto i : stack) { m = &m->at(i).children; }
			return m;
		}

		void activate() {
			auto m     = getcurrentmenu();
			auto &item = m->at(current);

			if(item.control) {
				if(item.control->activate()) {
					render(current, true);
					if(item.fn(item.control->get())) {
						auto assetM = engine->get_system<asset>().lock();
						IDType soundID;
						soundDtors[soundIndex++] = engine->add_object(&soundID);
						soundIndex %= soundDtors.size();
						engine->add_component<component::audiosource>(
						    soundID, assetM->get<polar::asset::audio>("menu1"),
						    support::audio::sourcetype::effect);
					}
				}
			} else if(!item.children.empty()) {
				navigate(0, 1, true);
			}
		}

		void navigate(int down, int right = 0, bool force = false) {
			auto m     = getcurrentmenu();
			auto &item = m->at(current);

			bool playBeep = false;

			bool newForce = force;
			if(!force && right && item.control &&
			   item.control->navigate(right)) {
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
				auto assetM = engine->get_system<asset>().lock();
				IDType soundID;
				soundDtors[soundIndex++] = engine->add_object(&soundID);
				soundIndex %= soundDtors.size();
				engine->add_component<component::audiosource>(
				    soundID, assetM->get<polar::asset::audio>("menu1"),
				    support::audio::sourcetype::effect);
			}
		}

		void render(size_t i, bool replace = false) {
			auto m     = getcurrentmenu();
			auto &item = m->at(i);

			IDType id;
			if(replace) {
				itemDtors.at(i) = engine->add_object(&id);
			} else {
				itemDtors.emplace_back(engine->add_object(&id));
			}

			const Decimal uiHeight     = 2.25;
			const Decimal uiTextHeight = 160;
			const Decimal uiTextWidth  = 550;

			/* max 6 items on screen at max scale of 0.375
			* 6 * 0.375 = 2.25 numerator
			*/
			Decimal scale = glm::min(Decimal(uiHeight) / Decimal(m->size()),
			                         Decimal(uiScale));
			Decimal spacing = uiTextHeight * scale;
			Point2 origin   = Point2(60, 50 + spacing * (m->size() - i - 1));

			engine->add_component<component::text>(id, font, item.value);
			engine->add_component<component::screenposition>(id, origin);
			engine->add_component<component::scale>(id, Point3(scale));

			auto text = engine->get_component<component::text>(id);

			if(i == current) {
				engine->add_component<component::color>(
				    id, Point4(1, 1, selectionAlpha, 1));
			}

			if(item.control) {
				IDType controlID;
				auto offset = Point2(uiTextWidth / uiScale * scale, 0);
				offset.y -= 12 * scale;
				controlDtors[i] = item.control->render(
				    engine, controlID, origin + offset, 8 * scale);
			}
		}

	  protected:
		void init() override final {
			auto assetM = engine->get_system<asset>().lock();
			auto inputM = engine->get_system<input>().lock();
			auto tw     = engine->get_system<tweener<float>>().lock();

			assetM->request<polar::asset::audio>("menu1");

			font = assetM->get<polar::asset::font>("nasalization-rg");

			for(auto k : {key_t::Down, key_t::S}) {
				dtors.emplace_back(
				    inputM->on(k, [this](key_t) { navigate(1); }));
			}

			for(auto k : {key_t::Up, key_t::W}) {
				dtors.emplace_back(
				    inputM->on(k, [this](key_t) { navigate(-1); }));
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
				dtors.emplace_back(
				    inputM->on(k, [this](key_t) { activate(); }));
			}

			for(auto k : {key_t::Escape, key_t::Backspace, key_t::MouseRight,
			              key_t::ControllerBack}) {
				dtors.emplace_back(
				    inputM->on(k, [this](key_t) { navigate(0, -1, true); }));
			}

			dtors.emplace_back(
			    inputM->onmousewheel([this](const Point2 &delta) {
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
			              [this](core::polar *engine, const float &x) {
				              selectionAlpha = x;
				          }));

			render_all();
		}

		void update(DeltaTicks &) override final { render(current, true); }

	  public:
		Decimal uiScale = 0.3125;

		static bool supported() { return true; }
		menu(core::polar *engine, Decimal uiScale, menuitem_vector_t _menu)
		    : base(engine), uiScale(uiScale), _menu(_menu) {}

		void render_all() {
			auto m = getcurrentmenu();

			itemDtors.clear();
			controlDtors.clear();
			for(size_t i = 0; i < m->size(); ++i) { render(i); }
		}
	};
}
}
