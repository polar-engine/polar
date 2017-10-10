#pragma once

#include <polar/component/screenposition.h>
#include <polar/component/text.h>
#include <polar/support/input/key.h>
#include <polar/support/ui/credits.h>
#include <polar/system/base.h>
#include <vector>

namespace polar {
namespace system {
	using credits_vector_t = std::vector<support::ui::credits_section>;

	class credits : public base {
		using key_t    = support::input::key;
		using origin_t = support::ui::origin;

	  private:
		credits_vector_t _credits;
		std::shared_ptr<polar::asset::font> font;
		Decimal height = 0;

		void render_all() {
			const Decimal pad     = 75;
			const Decimal forepad = 720;
			height                = -pad + forepad;

			for(auto &section : _credits) {
				height += pad;
				dtors.emplace_back(engine->add_object(&section.id));
				engine->add_component<component::text>(section.id, font,
				                                       section.value);
				engine->add_component<component::screenposition>(
				    section.id, Point2(0, height), origin_t::top);
				engine->add_component<component::scale>(section.id,
				                                        Point3(0.3125));
				height += Decimal(0.3125 * 1.12) * font->lineSkip;

				for(size_t n = 0; n < section.names.size(); ++n) {
					auto &name = section.names[n];
					dtors.emplace_back(engine->add_object(&section.nameIDs[n]));
					engine->add_component<component::text>(section.nameIDs[n],
					                                       font, name);
					engine->add_component<component::screenposition>(
					    section.nameIDs[n], Point2(0, height), origin_t::top);
					engine->add_component<component::scale>(section.nameIDs[n],
					                                        Point3(0.1875));
					height += Decimal(0.1875) * font->lineSkip;
				}
			}
		}

	  protected:
		void init() override final {
			auto inputM = engine->get_system<input>().lock();
			auto assetM = engine->get_system<asset>().lock();

			for(auto k : {key_t::Escape, key_t::Backspace, key_t::MouseRight,
			              key_t::ControllerBack}) {
				dtors.emplace_back(inputM->on(
				    k, [this](key_t) { engine->transition = "back"; }));
			}

			dtors.emplace_back(inputM->ondigital(
			    "menu_back", [this]() { engine->transition = "back"; }));

			font = assetM->get<polar::asset::font>("nasalization-rg");

			render_all();
		}

		void update(DeltaTicks &dt) override final {
			Decimal delta = dt.Seconds() * 50;

			for(auto &section : _credits) {
				auto sectionText =
				    engine->get_component<component::text>(section.id);
				auto sectionPos =
				    engine->get_component<component::screenposition>(
				        section.id);
				auto sectionScale =
				    engine->get_component<component::scale>(section.id);

				auto sectionRealScale =
				    sectionText->as->lineSkip * sectionScale->sc.get().y;
				sectionPos->position->y =
				    glm::mod(sectionPos->position->y - delta + sectionRealScale,
				             height) -
				    sectionRealScale;

				for(auto nameID : section.nameIDs) {
					auto nameText =
					    engine->get_component<component::text>(nameID);
					auto namePos =
					    engine->get_component<component::screenposition>(
					        nameID);
					auto nameScale =
					    engine->get_component<component::scale>(nameID);

					auto nameRealScale =
					    nameText->as->lineSkip * nameScale->sc.get().y;
					namePos->position->y =
					    glm::mod(namePos->position->y - delta + nameRealScale,
					             height) -
					    nameRealScale;
				}
			}
		}

	  public:
		static bool supported() { return true; }
		credits(core::polar *engine, credits_vector_t _credits)
		    : base(engine), _credits(_credits) {}
	};
}
}
