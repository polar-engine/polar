#pragma once

#include <stdint.h>
#include <polar/system/base.h>
#include <polar/component/scale.h>
#include <polar/component/color.h>
#include <polar/component/text.h>
#include <polar/component/screenposition.h>
#include <polar/component/sprite/box.h>
#include <polar/component/sprite/slider.h>

namespace MenuControl {
	class Base {
	public:
		virtual ~Base() {}
		virtual float Get() { return 0; }
		virtual bool Activate() { return false; }
		virtual bool Navigate(int) { return false; }
		virtual std::shared_ptr<Destructor> Render(Polar *engine, IDType &id, Point2, float) {
			return engine->AddObject(&id);
		}
	};

	class Button : public Base {
	public:
		Button() {}
		bool Activate() override final { return true; }
	};

	class Checkbox : public Base {
	private:
		bool state;
	public:
		Checkbox(bool initial = false) : state(initial) {}
		float Get() override final { return state; }

		bool Activate() override final {
			state = !state;
			return true;
		}

		bool Navigate(int delta) override {
			// flip state delta times
			state ^= delta & 1;
			return true;
		}

		std::shared_ptr<Destructor> Render(Polar *engine, IDType &id, Point2 origin, float scale) override final {
			auto dtor = engine->AddObject(&id);

			Decimal pad = 15;
			Point2 offset = Point2(4 * scale, 0); // edgeOffset + edgePadding (SliderSprite)s
			Point4 color = state ? Point4(0, 1, 0, 1) : Point4(1, 0, 0, 1);

			engine->AddComponentAs<Sprite, BoxSprite>(id);
			engine->AddComponent<ScreenPositionComponent>(id, origin + pad + offset);
			engine->AddComponent<ScaleComponent>(id, Point3(scale));
			engine->AddComponent<ColorComponent>(id, color);

			return dtor;
		}
	};

	template<typename T> class Slider : public Base {
	private:
		T min;
		T max;
		T value;
		T step;
	public:
		Slider(T min, T max, T initial = 0, T step = 1) : min(min), max(max), value(initial), step(step) {}
		float Get() override final { return value; }
		bool Activate() override final { return Navigate(1); }

		bool Navigate(int delta) override final {
			T newValue = glm::clamp(value + T(delta) * step, min, max);
			bool changed = newValue != value;
			value = newValue;
			return changed;
		}

		std::shared_ptr<Destructor> Render(Polar *engine, IDType &id, Point2 origin, float scale) override final {
			auto dtor = engine->AddObject(&id);

			Decimal pad = 15;
			float alpha = float(value - min) / float(max - min);

			engine->AddComponentAs<Sprite, SliderSprite>(id, 12 * 8, 12, alpha);
			engine->AddComponent<ScreenPositionComponent>(id, origin + pad);
			engine->AddComponent<ScaleComponent>(id, Point3(scale));

			return dtor;
		}
	};
}

class MenuItem {
public:
	using FTy = bool(float);

	std::string value;
	std::vector<MenuItem> children = {};
	std::function<FTy> fn;
	std::shared_ptr<MenuControl::Base> control;

	MenuItem(std::string value, std::vector<MenuItem> children) : value(value), children(children) {}
	MenuItem(std::string value, std::function<FTy> fn) : MenuItem(value, MenuControl::Button(), fn) {}
	template<typename T> MenuItem(std::string value, T control, std::function<FTy> fn) : value(value), fn(fn) {
		static_assert(std::is_base_of<MenuControl::Base, T>::value, "MenuItem requires object of base class MenuControl::Base");
		this->control = std::shared_ptr<MenuControl::Base>(new T(control));
	}
};

using Menu = std::vector<MenuItem>;

class MenuSystem : public System {
private:
	Menu menu;
	std::vector<size_t> stack;
	int current = 0;

	std::shared_ptr<FontAsset> font;
	std::vector<std::shared_ptr<Destructor>> itemDtors;
	std::unordered_map<int, std::shared_ptr<Destructor>> controlDtors;
	float selectionAlpha = 0.0f;

	// the size of the array determines how many concurrent sounds we can play at once
	std::array<std::shared_ptr<Destructor>, 4> soundDtors;
	size_t soundIndex = 0;

	Menu * GetCurrentMenu() {
		Menu *m = &menu;
		for(auto i : stack) {
			m = &m->at(i).children;
		}
		return m;
	}

	void Activate() {
		auto m = GetCurrentMenu();
		auto &item = m->at(current);

		if(item.control) {
			if(item.control->Activate()) {
				Render(current, true);
				if(item.fn(item.control->Get())) {
					auto assetM = engine->GetSystem<AssetManager>().lock();
					IDType soundID;
					soundDtors[soundIndex++] = engine->AddObject(&soundID);
					soundIndex %= soundDtors.size();
					engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>("menu1"), AudioSourceType::Effect);
				}
			}
		} else if(!item.children.empty()) { Navigate(0, 1, true); }
	}

	void Navigate(int down, int right = 0, bool force = false) {
		auto m = GetCurrentMenu();
		auto &item = m->at(current);

		bool playBeep = false;

		bool newForce = force;
		if(!force && right && item.control && item.control->Navigate(right)) {
			item.fn(item.control->Get());
			Render(current, true);
			playBeep = true;
		} else { newForce = force; }

		if(newForce) {
			while(right > 0) {
				auto m = GetCurrentMenu();
				auto &i = m->at(current);
				if(!i.children.empty()) {
					stack.emplace_back(current);
					current = 0;
					--right;
					playBeep = true;
				} else {
					Activate();
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
			RenderAll();
		}
		if(down != 0) {
			auto m = GetCurrentMenu();
			auto previous = current;
			current += down;
			if(current < 0) { current += m->size(); } else { current %= m->size(); }
			Render(previous, true);
			playBeep = true;
		}

		if(playBeep) {
			auto assetM = engine->GetSystem<AssetManager>().lock();
			IDType soundID;
			soundDtors[soundIndex++] = engine->AddObject(&soundID);
			soundIndex %= soundDtors.size();
			engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>("menu1"), AudioSourceType::Effect);
		}
	}

	void Render(size_t i, bool replace = false) {
		auto m = GetCurrentMenu();
		auto &item = m->at(i);

		IDType id;
		if(replace) {
			itemDtors.at(i) = engine->AddObject(&id);
		} else {
			itemDtors.emplace_back(engine->AddObject(&id));
		}

		const Decimal uiHeight = 2.25;
		const Decimal uiTextHeight = 160;
		const Decimal uiTextWidth = 550;

		/* max 6 items on screen at max scale of 0.375
		* 6 * 0.375 = 2.25 numerator
		*/
		Decimal scale = glm::min(Decimal(uiHeight) / Decimal(m->size()), Decimal(uiScale));
		Decimal spacing = uiTextHeight * scale;
		Point2 origin = Point2(60, 50 + spacing * (m->size() - i - 1));

		engine->AddComponent<Text>(id, font, item.value);
		engine->AddComponent<ScreenPositionComponent>(id, origin);
		engine->AddComponent<ScaleComponent>(id, Point3(scale));

		auto text = engine->GetComponent<Text>(id);

		if(i == current) {
			engine->AddComponent<ColorComponent>(id, Point4(1, 1, selectionAlpha, 1));
		}

		if(item.control) {
			IDType controlID;
			auto offset = Point2(uiTextWidth / uiScale * scale, 0);
			offset.y -= 12 * scale;
			controlDtors[i] = item.control->Render(engine, controlID, origin + offset, 8 * scale);
		}
	}
protected:
	void Init() override final {
		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();

		assetM->Request<AudioAsset>("menu1");

		font = assetM->Get<FontAsset>("nasalization-rg");

		for(auto k : { Key::Down, Key::S }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Navigate(1); }));
		}

		for(auto k : { Key::Up, Key::W }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Navigate(-1); }));
		}

		for(auto k : { Key::Left, Key::A }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Navigate(0, -1); }));
		}

		for(auto k : { Key::Right, Key::D }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Navigate(0, 1); }));
		}

		for(auto k : { Key::Space, Key::Enter, Key::MouseLeft, Key::ControllerA }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Activate(); }));
		}

		for(auto k : { Key::Escape, Key::Backspace, Key::MouseRight, Key::ControllerBack }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Navigate(0, -1, true); }));
		}

		dtors.emplace_back(inputM->OnMouseWheel([this] (const Point2 &delta) { Navigate(int(-delta.y), int(delta.x)); }));

		dtors.emplace_back(inputM->OnDigital("menu_up",      [this] () { Navigate(-1); }));
		dtors.emplace_back(inputM->OnDigital("menu_down",    [this] () { Navigate( 1); }));
		dtors.emplace_back(inputM->OnDigital("menu_left",    [this] () { Navigate( 0, -1); }));
		dtors.emplace_back(inputM->OnDigital("menu_right",   [this] () { Navigate( 0,  1); }));
		dtors.emplace_back(inputM->OnDigital("menu_confirm", [this] () { Activate(); }));
		dtors.emplace_back(inputM->OnDigital("menu_back",    [this] () { Navigate( 0, -1, true); }));

		dtors.emplace_back(tweener->Tween(0.0f, 1.0f, 0.25, true, [this] (Polar *engine, const float &x) {
			selectionAlpha = x;
		}));

		RenderAll();
	}

	void Update(DeltaTicks &) override final {
		Render(current, true);
	}
public:
	Decimal uiScale = 0.3125;

	static bool IsSupported() { return true; }
	MenuSystem(Polar *engine, Decimal uiScale, Menu menu) : System(engine), uiScale(uiScale), menu(menu) {}

	void RenderAll() {
		auto m = GetCurrentMenu();

		itemDtors.clear();
		controlDtors.clear();
		for(size_t i = 0; i < m->size(); ++i) {
			Render(i);
		}
	}
};
