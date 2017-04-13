#pragma once

#include "System.h"
#include <stdint.h>

template<typename T, typename FTy> struct RoseTree {
	T value;
	std::vector<RoseTree<T, FTy>> children;
	std::function<FTy> fn;

	RoseTree(T value, std::function<FTy> fn) : value(value), fn(fn) {}
	RoseTree(T value, std::vector<RoseTree<T, FTy>> children) : value(value), children(children) {}
};

using MenuItem = RoseTree<std::string, void()>;
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

	void Transition() {
		auto m = GetCurrentMenu();
		auto &item = m->at(current);

		if(item.children.empty()) {
			item.fn();
		} else {
			Navigate(0, 1);
		}
		/*switch(current) {
		case 0:
			engine->transition = "forward";
		case 1:
			stack.emplace_back(1);
			break;
		case 2:
			engine->Quit();
			break;
		}*/
	}

	void Navigate(int down, int right = 0) {
		while(right > 0) {
			stack.emplace_back(current);
			current = 0;
			--right;
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

	Menu * GetCurrentMenu() {
		Menu *m = &menu;
		for(auto i : stack) {
			m = &m->at(i).children;
		}
		return m;
	}
protected:
	void Init() override final {
		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();

		font = assetM->Get<FontAsset>("nasalization-rg");

		for(auto k : { Key::Escape, Key::ControllerBack }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { Navigate(0, -1); }));
		}

		dtors.emplace_back(inputM->On(Key::Down, [this] (Key) { Navigate(1); }));
		dtors.emplace_back(inputM->On(Key::S,    [this] (Key) { Navigate(1); }));

		dtors.emplace_back(inputM->On(Key::Up, [this] (Key) { Navigate(-1); }));
		dtors.emplace_back(inputM->On(Key::W,  [this] (Key) { Navigate(-1); }));

		dtors.emplace_back(tweener->Tween(0.0f, 1.0f, 0.25, true, [this] (Polar *engine, const float &x) {
			selectionAlpha = x;
		}));

		dtors.emplace_back(inputM->On(Key::Space,       [this] (Key) { Transition(); }));
		dtors.emplace_back(inputM->On(Key::Enter,       [this] (Key) { Transition(); }));
		dtors.emplace_back(inputM->On(Key::ControllerA, [this] (Key) { Transition(); }));
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
