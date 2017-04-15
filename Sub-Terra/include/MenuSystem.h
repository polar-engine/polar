#pragma once

#include "System.h"
#include <stdint.h>

namespace MenuControl {
	class Base {
	public:
		virtual ~Base() {}
		virtual float Get() { return 0; }
		virtual void Activate() {}
		virtual bool Navigate(int) { return false; }
	};

	class Button : public Base {
	public:
		Button() {}
	};

	class Checkbox : public Base {
	private:
		bool state;
	public:
		Checkbox(bool initial = false) : state(initial) {}

		float Get() override final { return state; }

		void Activate() override final {
			state = !state;
		}
	};

	template<typename T> class Slider : public Base {
	private:
		T min;
		T max;
		T value;
	public:
		Slider(T min, T max, T initial = 0) : min(min), max(max), value(initial) {}

		float Get() override final { return value; }

		bool Navigate(int delta) {
			T newValue = glm::clamp(value + T(delta), min, max);
			bool changed = newValue != value;
			value = newValue;
			return changed;
		}
	};

	class Selection : public Base {
	public:
		Selection(std::vector<std::string> options) {}
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
	std::vector<boost::shared_ptr<Destructor>> itemDtors;
	float selectionAlpha = 0.0f;

	// the size of the array determines how many concurrent sounds we can play at once
	std::array<boost::shared_ptr<Destructor>, 4> soundDtors;
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
			item.control->Activate();
			if(item.fn(item.control->Get())) {
				auto assetM = engine->GetSystem<AssetManager>().lock();
				IDType soundID;
				soundDtors[soundIndex++] = engine->AddObject(&soundID);
				soundIndex %= soundDtors.size();
				engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>("menu1"));
			}
		} else if(!item.children.empty()) {
			Navigate(0, 1);
		}
	}

	void Navigate(int down, int right = 0, bool force = false) {
		auto m = GetCurrentMenu();
		auto &item = m->at(current);

		if(!force && right && item.control && item.control->Navigate(right)) {
			item.fn(item.control->Get());
		} else { force = true; }

		if(force) {
			while(right > 0) {
				auto m = GetCurrentMenu();
				auto &i = m->at(current);
				if(!i.children.empty()) {
					stack.emplace_back(current);
					current = 0;
					--right;
				} else {
					Activate();
					return;
				}
			}
			while(right < 0) {
				if(stack.empty()) {
					engine->transition = "back";
					return;
				} else {
					current = stack.back();
					stack.pop_back();
					++right;
				}
			}
		}
		if(down != 0) {
			auto m = GetCurrentMenu();
			current += down;
			if(current < 0) { current += m->size(); } else { current %= m->size(); }
		}

		auto assetM = engine->GetSystem<AssetManager>().lock();
		IDType soundID;
		soundDtors[soundIndex++] = engine->AddObject(&soundID);
		soundIndex %= soundDtors.size();
		engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>("menu1"));
	}
protected:
	void Init() override final {
		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();

		font = assetM->Get<FontAsset>("nasalization-rg");

		for(auto k : { Key::Escape, Key::Backspace, Key::ControllerBack }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Navigate(0, -1, true); }));
		}

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

		for(auto k : { Key::Space, Key::Enter, Key::ControllerA }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Activate(); }));
		}

		dtors.emplace_back(tweener->Tween(0.0f, 1.0f, 0.25, true, [this] (Polar *engine, const float &x) {
			selectionAlpha = x;
		}));
	}

	void Update(DeltaTicks &) override final {
		auto m = GetCurrentMenu();

		itemDtors.clear();
		for(size_t i = 0; i < m->size(); ++i) {
			auto &item = m->at(i);
			IDType id;
			itemDtors.emplace_back(engine->AddObject(&id));
			engine->AddComponent<Text>(id, font, item.value, Point2(60, 50 + 60 * (m->size() - i - 1)));
			auto t = engine->GetComponent<Text>(id);
			t->scale *= 0.375f;
			if(i == current) {
				t->color.b = selectionAlpha;
			}
		}
	}
public:
	static bool IsSupported() { return true; }
	MenuSystem(Polar *engine, Menu menu) : System(engine), menu(menu) {}
};
