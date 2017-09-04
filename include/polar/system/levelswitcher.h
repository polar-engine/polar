#pragma once

#include <vector>
#include <polar/system/base.h>
#include <polar/asset/level.h>
#include <polar/component/scale.h>
#include <polar/component/color.h>
#include <polar/component/screenposition.h>

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
		auto qIndex = GetIndex(-1);
		auto eIndex = GetIndex( 1);

		engine->GetComponent<ColorComponent>(qID)->color = Point4(GetLevel(qIndex)->keyframes.begin()->colors[0], 1);
		engine->GetComponent<ColorComponent>(eID)->color = Point4(GetLevel(eIndex)->keyframes.begin()->colors[0], 1);
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

		engine->AddComponent<Text>(qID, font, "Q");
		engine->AddComponent<Text>(eID, font, "E");

		engine->AddComponent<ScreenPositionComponent>(qID, Point2(20), Origin::TopLeft);
		engine->AddComponent<ScreenPositionComponent>(eID, Point2(20), Origin::TopRight);

		engine->AddComponent<ColorComponent>(qID, Point4(0.8902, 0.9647, 0.9922, 0));
		engine->AddComponent<ColorComponent>(eID, Point4(0.8902, 0.9647, 0.9922, 0));

		engine->AddComponent<ScaleComponent>(qID, Point3(0.5));
		engine->AddComponent<ScaleComponent>(eID, Point3(0.5));

		auto inputM = engine->GetSystem<InputManager>().lock();
		dtors.emplace_back(inputM->On(Key::Q, [this] (Key) { UpdateIndex(-1); }));
		dtors.emplace_back(inputM->On(Key::E, [this] (Key) { UpdateIndex( 1); }));
		dtors.emplace_back(inputM->OnDigital("menu_previous", [this] () { UpdateIndex(-1); }));
		dtors.emplace_back(inputM->OnDigital("menu_next",     [this] () { UpdateIndex( 1); }));

		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		dtors.emplace_back(tweener->Tween(0, 1, 0.5, true, [this] (Polar *engine, const float &x) {
			auto alpha = enabled ? glm::pow(Decimal(x), Decimal(0.75)) : 0;
			engine->GetComponent<ColorComponent>(qID)->color->a = alpha;
			engine->GetComponent<ColorComponent>(eID)->color->a = alpha;
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
