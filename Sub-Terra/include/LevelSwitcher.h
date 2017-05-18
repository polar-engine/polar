#pragma once

#include <vector>
#include "System.h"
#include "Level.h"

class LevelSwitcher : public System {
private:
	std::vector<std::string> levelNames;
	int levelIndex = 0;
	IDType qID = INVALID_ID();
	IDType eID = INVALID_ID();
	bool enabled = true;

	int GetIndex(int delta) {
		int i = levelIndex + delta;
		while(i < 0) { i += levelNames.size(); }
		return i % levelNames.size();
	}

	void UpdateQE() {
		auto assetM = engine->GetSystem<AssetManager>().lock();

		auto qIndex = GetIndex(-1);
		auto eIndex = GetIndex( 1);

		engine->GetComponent<Sprite>(qID)->color = Point4(GetLevel(qIndex)->keyframes.begin()->colors[0], 1);
		engine->GetComponent<Sprite>(eID)->color = Point4(GetLevel(eIndex)->keyframes.begin()->colors[0], 1);
	}

	void UpdateIndex(int delta) {
		if(enabled) {
			levelIndex = GetIndex(delta);
			engine->GetSystem<World>().lock()->SetLevel(GetLevel());
			UpdateQE();
		}
	}
protected:
	void Init() override final {
		auto assetM = engine->GetSystem<AssetManager>().lock();

		levelNames = assetM->List<Level>();
		levelIndex = 0;

		dtors.emplace_back(engine->AddObject(&qID));
		dtors.emplace_back(engine->AddObject(&eID));

		auto font = assetM->Get<FontAsset>("nasalization-rg");
		engine->AddComponentAs<Sprite, Text>(qID, font, "Q", Point2(20), Origin::TopLeft,  Point4(0.8902, 0.9647, 0.9922, 0));
		engine->AddComponentAs<Sprite, Text>(eID, font, "E", Point2(20), Origin::TopRight, Point4(0.8902, 0.9647, 0.9922, 0));

		engine->GetComponent<Sprite>(qID)->scale *= 0.5;
		engine->GetComponent<Sprite>(eID)->scale *= 0.5;

		auto inputM = engine->GetSystem<InputManager>().lock();
		dtors.emplace_back(inputM->On(Key::Q, [this] (Key) { UpdateIndex(-1); }));
		dtors.emplace_back(inputM->On(Key::E, [this] (Key) { UpdateIndex( 1); }));
		dtors.emplace_back(inputM->OnDigital("menu_previous", [this] () { UpdateIndex(-1); }));
		dtors.emplace_back(inputM->OnDigital("menu_next",     [this] () { UpdateIndex( 1); }));

		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		dtors.emplace_back(tweener->Tween(0, 1, 0.5, true, [this] (Polar *engine, const float &x) {
			auto alpha = enabled ? glm::pow(Decimal(x), Decimal(0.75)) : 0;
			engine->GetComponent<Sprite>(qID)->color.a = alpha;
			engine->GetComponent<Sprite>(eID)->color.a = alpha;
		}));

		UpdateQE();
	}
public:
	static bool IsSupported() { return true; }
	LevelSwitcher(Polar *engine) : System(engine) {}

	void SetEnabled(bool e) {
		enabled = e;
	}

	std::shared_ptr<Level> GetLevel() { return GetLevel(levelIndex); }

	std::shared_ptr<Level> GetLevel(int i) {
		auto assetM = engine->GetSystem<AssetManager>().lock();
		return assetM->Get<Level>(levelNames[i]);
	}
};
