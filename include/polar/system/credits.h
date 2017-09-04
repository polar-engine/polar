#pragma once

#include <vector>
#include <polar/system/base.h>
#include <polar/component/text.h>
#include <polar/component/screenposition.h>

class CreditsSection {
public:
	std::string value;
	std::vector<std::string> names;
	IDType id = INVALID_ID();
	std::vector<IDType> nameIDs;

	CreditsSection(std::string value, std::vector<std::string> names) : value(value), names(names) {
		for(size_t i = 0; i < names.size(); ++i) { nameIDs.emplace_back(INVALID_ID()); }
	}
};

using Credits = std::vector<CreditsSection>;

class CreditsSystem : public System {
private:
	Credits credits;
	std::shared_ptr<FontAsset> font;
	Decimal height = 0;

	void RenderAll() {
		const Decimal pad = 75;
		const Decimal forepad = 720;
		height = -pad + forepad;

		for(auto &section : credits) {
			height += pad;
			dtors.emplace_back(engine->AddObject(&section.id));
			engine->AddComponent<Text>(section.id, font, section.value);
			engine->AddComponent<ScreenPositionComponent>(section.id, Point2(0, height), Origin::Top);
			engine->AddComponent<ScaleComponent>(section.id, Point3(0.3125));
			height += Decimal(0.3125 * 1.12) * font->lineSkip;

			for(size_t n = 0; n < section.names.size(); ++n) {
				auto &name = section.names[n];
				dtors.emplace_back(engine->AddObject(&section.nameIDs[n]));
				engine->AddComponent<Text>(section.nameIDs[n], font, name);
				engine->AddComponent<ScreenPositionComponent>(section.nameIDs[n], Point2(0, height), Origin::Top);
				engine->AddComponent<ScaleComponent>(section.nameIDs[n], Point3(0.1875));
				height += Decimal(0.1875) * font->lineSkip;
			}
		}
	}
protected:
	void Init() override final {
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto assetM = engine->GetSystem<AssetManager>().lock();

		for(auto k : { Key::Escape, Key::Backspace, Key::MouseRight, Key::ControllerBack }) {
			dtors.emplace_back(inputM->On(k, [this] (Key) { engine->transition = "back"; }));
		}

		dtors.emplace_back(inputM->OnDigital("menu_back", [this] () { engine->transition = "back"; }));

		font = assetM->Get<FontAsset>("nasalization-rg");

		RenderAll();
	}

	void Update(DeltaTicks &dt) override final {
		Decimal delta = dt.Seconds() * 50;

		for(auto &section : credits) {
			auto sectionText = engine->GetComponent<Text>(section.id);
			auto sectionPos = engine->GetComponent<ScreenPositionComponent>(section.id);
			auto sectionScale = engine->GetComponent<ScaleComponent>(section.id);

			auto sectionRealScale = sectionText->asset->lineSkip * sectionScale->scale.Get().y;
			sectionPos->position->y = glm::mod(sectionPos->position->y - delta + sectionRealScale, height) - sectionRealScale;

			for(auto nameID : section.nameIDs) {
				auto nameText = engine->GetComponent<Text>(nameID);
				auto namePos = engine->GetComponent<ScreenPositionComponent>(nameID);
				auto nameScale = engine->GetComponent<ScaleComponent>(nameID);

				auto nameRealScale = nameText->asset->lineSkip * nameScale->scale.Get().y;
				namePos->position->y = glm::mod(namePos->position->y - delta + nameRealScale, height) - nameRealScale;
			}
		}
	}
public:
	static bool IsSupported() { return true; }
	CreditsSystem(Polar *engine, Credits credits) : System(engine), credits(credits) {}
};
