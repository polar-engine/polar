#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/text.h>
#include <polar/support/action/credits.h>
#include <polar/system/asset.h>
#include <polar/system/credits.h>

namespace polar::system {
	void credits::render_all() {
		const Decimal pad     = 75;
		const Decimal forepad = 720;
		height                = -pad + forepad;

		for(auto &section : _credits) {
			height += pad;
			keep(engine->add(section.id));
			engine->add<component::text>(section.id, font, section.value);
			engine->add<component::screenposition>(
			    section.id, Point2(0, height), origin_t::top);
			engine->add<component::scale>(section.id, Point3(0.3125));
			height += Decimal(0.3125 * 1.12) * font->lineSkip;

			for(size_t n = 0; n < section.names.size(); ++n) {
				auto &name = section.names[n];
				keep(engine->add(section.nameIDs[n]));
				engine->add<component::text>(section.nameIDs[n], font, name);
				engine->add<component::screenposition>(
				    section.nameIDs[n], Point2(0, height), origin_t::top);
				engine->add<component::scale>(section.nameIDs[n],
				                              Point3(0.1875));
				height += Decimal(0.1875) * font->lineSkip;
			}
		}
	}

	void credits::init() {
		using lifetime = support::action::lifetime;

		auto assetM = engine->get<asset>().lock();
		auto act = engine->get<action>().lock();

		if(act) {
			keep(act->bind<support::action::credits::back>(lifetime::on, [this](auto) {
				engine->transition = "back";
				return true;
			}));
		}

		font = assetM->get<polar::asset::font>("nasalization-rg");

		render_all();
	}

	void credits::update(DeltaTicks &dt) {
		Decimal delta = dt.Seconds() * 50;

		for(auto &section : _credits) {
			auto sectionText = engine->get<component::text>(section.id);
			auto sectionPos =
			    engine->get<component::screenposition>(section.id);
			auto sectionScale = engine->get<component::scale>(section.id);

			auto sectionRealScale =
			    sectionText->as->lineSkip * sectionScale->sc.get().y;
			sectionPos->position->y =
			    glm::mod(sectionPos->position->y - delta + sectionRealScale,
			             height) -
			    sectionRealScale;

			for(auto nameID : section.nameIDs) {
				auto nameText  = engine->get<component::text>(nameID);
				auto namePos   = engine->get<component::screenposition>(nameID);
				auto nameScale = engine->get<component::scale>(nameID);

				auto nameRealScale =
				    nameText->as->lineSkip * nameScale->sc.get().y;
				namePos->position->y =
				    glm::mod(namePos->position->y - delta + nameRealScale,
				             height) -
				    nameRealScale;
			}
		}
	}
} // namespace polar::system
