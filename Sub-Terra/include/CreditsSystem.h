#pragma once

#include <vector>
#include "System.h"
#include "Text.h"

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
			engine->AddComponentAs<Sprite, Text>(section.id, font, section.value, Point2(0, height), Origin::Top);
			auto sectionSprite = engine->GetComponent<Sprite>(section.id);
			sectionSprite->scale *= 0.3125;
			height += sectionSprite->scale.y * 1.12;

			for(size_t n = 0; n < section.names.size(); ++n) {
				auto &name = section.names[n];
				dtors.emplace_back(engine->AddObject(&section.nameIDs[n]));
				engine->AddComponentAs<Sprite, Text>(section.nameIDs[n], font, name, Point2(0, height), Origin::Top);
				auto nameSprite = engine->GetComponent<Sprite>(section.nameIDs[n]);
				nameSprite->scale *= 0.1875;
				height += nameSprite->scale.y;
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

		font = assetM->Get<FontAsset>("nasalization-rg");

		RenderAll();
	}

	void Update(DeltaTicks &dt) override final {
		Decimal delta = dt.Seconds() * 50;

		for(auto &section : credits) {
			auto sectionSprite = engine->GetComponent<Sprite>(section.id);
			sectionSprite->position.y = glm::mod(sectionSprite->position.y - delta + sectionSprite->scale.y, height) - sectionSprite->scale.y;

			for(auto nameID : section.nameIDs) {
				auto nameSprite = engine->GetComponent<Sprite>(nameID);
				nameSprite->position.y = glm::mod(nameSprite->position.y - delta + nameSprite->scale.y, height) - nameSprite->scale.y;
			}
		}
	}
public:
	static bool IsSupported() { return true; }
	CreditsSystem(Polar *engine, Credits credits) : System(engine), credits(credits) {}
};
