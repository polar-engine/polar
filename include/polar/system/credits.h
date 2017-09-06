#pragma once

#include <vector>
#include <polar/system/base.h>
#include <polar/component/text.h>
#include <polar/component/screenposition.h>
#include <polar/support/ui/credits.h>
#include <polar/support/input/key.h>

namespace polar { namespace system {
	using credits_vector_t = std::vector<support::ui::credits_section>;

	class credits : public base {
		using key_t = support::input::key;
		using origin_t = support::ui::origin;
	private:
		credits_vector_t _credits;
		std::shared_ptr<polar::asset::font> font;
		Decimal height = 0;

		void render_all() {
			const Decimal pad = 75;
			const Decimal forepad = 720;
			height = -pad + forepad;

			for(auto &section : _credits) {
				height += pad;
				dtors.emplace_back(engine->addobject(&section.id));
				engine->addcomponent<component::text>(section.id, font, section.value);
				engine->addcomponent<component::screenposition>(section.id, Point2(0, height), origin_t::top);
				engine->addcomponent<component::scale>(section.id, Point3(0.3125));
				height += Decimal(0.3125 * 1.12) * font->lineSkip;

				for(size_t n = 0; n < section.names.size(); ++n) {
					auto &name = section.names[n];
					dtors.emplace_back(engine->addobject(&section.nameIDs[n]));
					engine->addcomponent<component::text>(section.nameIDs[n], font, name);
					engine->addcomponent<component::screenposition>(section.nameIDs[n], Point2(0, height), origin_t::top);
					engine->addcomponent<component::scale>(section.nameIDs[n], Point3(0.1875));
					height += Decimal(0.1875) * font->lineSkip;
				}
			}
		}
	protected:
		void init() override final {
			auto inputM = engine->getsystem<input>().lock();
			auto assetM = engine->getsystem<asset>().lock();

			for(auto k : { key_t::Escape, key_t::Backspace, key_t::MouseRight, key_t::ControllerBack }) {
				dtors.emplace_back(inputM->on(k, [this] (key_t) { engine->transition = "back"; }));
			}

			dtors.emplace_back(inputM->ondigital("menu_back", [this] () { engine->transition = "back"; }));

			font = assetM->get<polar::asset::font>("nasalization-rg");

			render_all();
		}

		void update(DeltaTicks &dt) override final {
			Decimal delta = dt.Seconds() * 50;

			for(auto &section : _credits) {
				auto sectionText  = engine->getcomponent<component::text>(section.id);
				auto sectionPos   = engine->getcomponent<component::screenposition>(section.id);
				auto sectionScale = engine->getcomponent<component::scale>(section.id);

				auto sectionRealScale = sectionText->as->lineSkip * sectionScale->sc.get().y;
				sectionPos->position->y = glm::mod(sectionPos->position->y - delta + sectionRealScale, height) - sectionRealScale;

				for(auto nameID : section.nameIDs) {
					auto nameText  = engine->getcomponent<component::text>(nameID);
					auto namePos   = engine->getcomponent<component::screenposition>(nameID);
					auto nameScale = engine->getcomponent<component::scale>(nameID);

					auto nameRealScale = nameText->as->lineSkip * nameScale->sc.get().y;
					namePos->position->y = glm::mod(namePos->position->y - delta + nameRealScale, height) - nameRealScale;
				}
			}
		}
	public:
		static bool supported() { return true; }
		credits(core::polar *engine, credits_vector_t _credits) : base(engine), _credits(_credits) {}
	};
} }
