#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/text.h>
#include <polar/support/action/credits.h>
#include <polar/system/asset.h>
#include <polar/system/credits.h>

namespace polar::system {
	void credits::render_all() {
		const math::decimal pad     = 75;
		const math::decimal forepad = 720;
		height                = -pad + forepad;

		for(auto &section : _credits) {
			height += pad;
			section.object = engine.add();
			engine.add<component::text>(section.object, font, section.value);
			engine.add<component::screenposition>(section.object, math::point2(0, height), origin_t::top);
			engine.add<component::scale>(section.object, math::point3(0.3125));
			height += math::decimal(0.3125 * 1.12) * font->lineSkip;

			for(size_t n = 0; n < section.names.size(); ++n) {
				auto &name = section.names[n];
				section.name_objects[n] = engine.add();
				engine.add<component::text>(section.name_objects[n], font, name);
				engine.add<component::screenposition>(section.name_objects[n], math::point2(0, height), origin_t::top);
				engine.add<component::scale>(section.name_objects[n], math::point3(0.1875));
				height += math::decimal(0.1875) * font->lineSkip;
			}
		}
	}

	void credits::init() {
		using lifetime = support::action::lifetime;

		auto assetM = engine.get<asset>().lock();
		auto act = engine.get<action>().lock();

		if(act) {
			keep(act->bind<support::action::credits::back>(lifetime::on, [this](auto) {
				engine.transition = "back";
			}));
		}

		font = assetM->get<polar::asset::font>("nasalization-rg");

		render_all();
	}

	void credits::update(DeltaTicks &dt) {
		math::decimal delta = dt.Seconds() * 50;

		for(auto &section : _credits) {
			auto sectionText = engine.get<component::text>(section.object);
			auto sectionPos = engine.mutate<component::screenposition>(section.object);
			auto sectionScale = engine.get<component::scale>(section.object);

			auto sectionRealScale = sectionText->as->lineSkip * sectionScale->sc.get().y;
			sectionPos->position->y = glm::mod(sectionPos->position->y - delta + sectionRealScale, height)
			                        - sectionRealScale;

			for(auto object : section.name_objects) {
				auto nameText  = engine.get<component::text>(object);
				auto namePos   = engine.mutate<component::screenposition>(object);
				auto nameScale = engine.get<component::scale>(object);

				auto nameRealScale = nameText->as->lineSkip * nameScale->sc.get().y;
				namePos->position->y = glm::mod(namePos->position->y - delta + nameRealScale, height) - nameRealScale;
			}
		}
	}
} // namespace polar::system
