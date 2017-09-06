#pragma once

#include <vector>
#include <polar/system/base.h>
#include <polar/system/tweener.h>
#include <polar/asset/level.h>
#include <polar/component/scale.h>
#include <polar/component/color.h>
#include <polar/component/screenposition.h>

namespace polar { namespace system {
	class levelswitcher : public base {
		using level_t = polar::asset::level;
		using key_t = support::input::key;
	private:
		std::vector<std::string> levelNames;
		int levelIndex = 0;
		IDType qID = INVALID_ID();
		IDType eID = INVALID_ID();
		bool enabled = true;

		int getindex(int delta) {
			int i = levelIndex + delta;
			while(i < 0) { i += levelNames.size(); }
			return i % levelNames.size();
		}

		void updateQE() {
			auto qIndex = getindex(-1);
			auto eIndex = getindex( 1);

			engine->getcomponent<component::color>(qID)->col = Point4(getlevel(qIndex)->keyframes.begin()->colors[0], 1);
			engine->getcomponent<component::color>(eID)->col = Point4(getlevel(eIndex)->keyframes.begin()->colors[0], 1);
		}

		void updateindex(int delta) {
			if(enabled) {
				levelIndex = getindex(delta);
				engine->getsystem<world>().lock()->set_level(getlevel());
				updateQE();
			}
		}
	protected:
		void init() override final {
			auto assetM = engine->getsystem<asset>().lock();

			levelNames = assetM->list<level_t>();
			levelIndex = 0;

			dtors.emplace_back(engine->addobject(&qID));
			dtors.emplace_back(engine->addobject(&eID));

			auto font = assetM->get<polar::asset::font>("nasalization-rg");

			engine->addcomponent<component::text>(qID, font, "Q");
			engine->addcomponent<component::text>(eID, font, "E");

			engine->addcomponent<component::screenposition>(qID, Point2(20), support::ui::origin::topleft);
			engine->addcomponent<component::screenposition>(eID, Point2(20), support::ui::origin::topright);

			engine->addcomponent<component::color>(qID, Point4(0.8902, 0.9647, 0.9922, 0));
			engine->addcomponent<component::color>(eID, Point4(0.8902, 0.9647, 0.9922, 0));

			engine->addcomponent<component::scale>(qID, Point3(0.5));
			engine->addcomponent<component::scale>(eID, Point3(0.5));

			auto inputM = engine->getsystem<input>().lock();
			dtors.emplace_back(inputM->on(key_t::Q, [this] (key_t) { updateindex(-1); }));
			dtors.emplace_back(inputM->on(key_t::E, [this] (key_t) { updateindex( 1); }));
			dtors.emplace_back(inputM->ondigital("menu_previous", [this] () { updateindex(-1); }));
			dtors.emplace_back(inputM->ondigital("menu_next",     [this] () { updateindex( 1); }));

			auto tw = engine->getsystem<tweener<float>>().lock();
			dtors.emplace_back(tw->tween(0, 1, 0.5, true, [this] (core::polar *engine, const float &x) {
				auto alpha = enabled ? glm::pow(Decimal(x), Decimal(0.75)) : 0;
				engine->getcomponent<component::color>(qID)->col->a = alpha;
				engine->getcomponent<component::color>(eID)->col->a = alpha;
			}));

			updateQE();
		}
	public:
		static bool supported() { return true; }
		levelswitcher(core::polar *engine) : base(engine) {}

		void setenabled(bool e) {
			enabled = e;
		}

		std::shared_ptr<level_t> getlevel() { return getlevel(levelIndex); }

		std::shared_ptr<level_t> getlevel(int i) {
			auto assetM = engine->getsystem<asset>().lock();
			return assetM->get<level_t>(levelNames[i]);
		}
	};
} }
