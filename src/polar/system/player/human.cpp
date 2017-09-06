#include <iomanip>
#include <glm/gtc/noise.hpp>
#include <polar/core/polar.h>
#include <polar/system/player/human.h>
#include <polar/system/asset.h>
#include <polar/system/event.h>
#include <polar/system/input.h>
#include <polar/system/world.h>
#include <polar/component/position.h>
#include <polar/component/screenposition.h>
#include <polar/component/color.h>
#include <polar/component/orientation.h>
#include <polar/component/playercamera.h>
#include <polar/component/bounds.h>
#include <polar/component/physical.h>
#include <polar/component/text.h>
#include <polar/component/audiosource.h>

namespace polar { namespace system { namespace player {
	void human::init() {
		engine->addcomponent<component::playercamera>(object);

		auto inputM = engine->getsystem<input>().lock();
		auto ownPos = engine->getcomponent<component::position>(object);
		auto ownBounds = engine->getcomponent<component::bounds>(object);
		auto camera = engine->getcomponent<component::playercamera>(object);

		camera->position = Point3(0.0f);

		/* mouse look */
		const float mouseSpeed = 0.015f;
		dtors.emplace_back(inputM->onmousemove([this, mouseSpeed] (const Point2 &delta) {
			Decimal smoothingVelFactor = glm::pow(Decimal(2) - smoothing, Decimal(40));
			orientVel.y += glm::radians(mouseSpeed) * delta.x * smoothingVelFactor;
			orientVel.x += glm::radians(mouseSpeed) * delta.y * smoothingVelFactor;
		}));
		dtors.emplace_back(inputM->oncontrolleraxes([this, mouseSpeed] (const Point2 &delta) {
			Decimal smoothingVelFactor = glm::pow(Decimal(2) - smoothing, Decimal(40));
			orientVel.y += glm::radians(mouseSpeed) * delta.x * 30.0f * smoothingVelFactor;
			orientVel.x += glm::radians(mouseSpeed) * delta.y * 30.0f * smoothingVelFactor;
		}));
		dtors.emplace_back(inputM->onanalog("ingame_camera", [this, mouseSpeed] (const Point2 &delta) {
			Decimal smoothingVelFactor = glm::pow(Decimal(2) - smoothing, Decimal(40));
			orientVel.y += glm::radians(mouseSpeed) * delta.x * smoothingVelFactor;
			orientVel.x += glm::radians(mouseSpeed) * delta.y * smoothingVelFactor;
		}));

		dtors.emplace_back(inputM->on(key_t::Space, [this] (key_t) {
			engine->getsystem<world>().lock()->turbo = true;
		}));
		dtors.emplace_back(inputM->after(key_t::Space, [this] (key_t) {
			engine->getsystem<world>().lock()->turbo = false;
		}));

		/* collision detection and response */
		dtors.emplace_back(engine->getsystem<event>().lock()->listenfor("integrator", "ticked", [this, ownPos, ownBounds] (support::event::arg delta) {
			auto wld = engine->getsystem<world>().lock();

			if(wld) {
				auto &curr = *ownPos->pos;
				if(wld->eval(curr)) {
					engine->transition = "gameover";
				}
			}
		}));
	}

	#define TIMEDSOUND(TIME, NAME)                                                                              \
		if(oldTime < TIME && time >= TIME) {                                                                    \
			soundDtors[soundIndex++] = engine->addobject(&soundID);                                             \
			soundIndex %= soundDtors.size();                                                                    \
			engine->addcomponent<component::audiosource>(soundID, assetM->get<polar::asset::audio>(NAME), support::audio::sourcetype::effect); \
		}

	void human::update(DeltaTicks &dt) {
		auto assetM = engine->getsystem<asset>().lock();
		auto inputM = engine->getsystem<input>().lock();
		auto wld = engine->getsystem<world>().lock();

		auto time = wld->get_ticks() / Decimal(ENGINE_TICKS_PER_SECOND);

		IDType soundID;
		TIMEDSOUND(  30.0f, "30");
		TIMEDSOUND(  30.75f, "seconds");
		TIMEDSOUND(  60.0f, "60");
		TIMEDSOUND(  60.9f, "seconds");
		TIMEDSOUND( 100.0f, "1");
		TIMEDSOUND( 100.7f, "hundred");
		TIMEDSOUND( 150.0f, "1");
		TIMEDSOUND( 150.7f, "fifty");
		TIMEDSOUND( 200.0f, "2");
		TIMEDSOUND( 200.7f, "hundred");
		TIMEDSOUND( 250.0f, "2");
		TIMEDSOUND( 250.7f, "fifty");
		TIMEDSOUND( 300.0f, "3");
		TIMEDSOUND( 300.7f, "hundred");
		TIMEDSOUND( 350.0f, "3");
		TIMEDSOUND( 350.7f, "fifty");
		TIMEDSOUND( 400.0f, "4");
		TIMEDSOUND( 400.85f, "hundred");
		TIMEDSOUND( 450.0f, "4");
		TIMEDSOUND( 450.85f, "fifty");
		TIMEDSOUND( 500.0f, "5");
		TIMEDSOUND( 500.85f, "hundred");
		TIMEDSOUND( 550.0f, "5");
		TIMEDSOUND( 550.85f, "fifty");
		TIMEDSOUND( 600.0f, "6");
		TIMEDSOUND( 600.9f, "hundred");
		TIMEDSOUND( 650.0f, "6");
		TIMEDSOUND( 650.9f, "fifty");
		TIMEDSOUND( 700.0f, "7");
		TIMEDSOUND( 700.85f, "hundred");
		TIMEDSOUND( 750.0f, "7");
		TIMEDSOUND( 750.85f, "fifty");
		TIMEDSOUND( 800.0f, "8");
		TIMEDSOUND( 800.6f, "hundred");
		TIMEDSOUND( 850.0f, "8");
		TIMEDSOUND( 850.6f, "fifty");
		TIMEDSOUND( 900.0f, "9");
		TIMEDSOUND( 900.85f, "hundred");
		TIMEDSOUND( 950.0f, "9");
		TIMEDSOUND( 950.85f, "fifty");
		TIMEDSOUND(1000.0f, "freefall");

		auto font = assetM->get<polar::asset::font>("nasalization-rg");
		timeDtor = engine->addobject(&timeID);

		std::ostringstream oss;
		oss << std::setiosflags(std::ios::fixed) << std::setprecision(1) << time << 's';

		Point3 color;
		if(time >= 100.0f) {
			color = Point3(1.0, 0.3,  0.1);
		} else if(time > 60.0f) {
			color = Point3(1.0, 0.75, 0.5);
		} else {
			color = Point3(0.7, 0.95, 1.0);
		}
		float alpha = glm::mix(0.8f, 0.35f, 1.0f - glm::abs(glm::pow(glm::sin((time - 0.5) * glm::pi<Decimal>()), 8.0)));

		engine->addcomponent<component::text>(timeID, font, oss.str());
		engine->addcomponent<component::screenposition>(timeID, Point2(20, 20), support::ui::origin::topright);
		engine->addcomponent<component::color>(timeID, Point4(color, alpha));

		auto ownPos = engine->getcomponent<component::position>(object);
		auto orient = engine->getcomponent<component::orientation>(object);
		auto camera = engine->getcomponent<component::playercamera>(object);

		orientVel *= static_cast<float>(glm::pow(smoothing, dt.Seconds() * Decimal(1000)));
		orient->orient = Quat(Point3(orientVel.x, 0.0, 0.0)) * Quat(Point3(0.0, orientVel.y, 0.0)) * orient->orient;

		Decimal seconds = engine->getsystem<world>().lock()->get_ticks() / Decimal(ENGINE_TICKS_PER_SECOND);
		const Decimal a = Decimal(1.32499);
		const Decimal r = Decimal(1.01146);
		const Decimal k = Decimal(1.66377);
		velocity = 10 + 40 * a * (1 - glm::pow(r, k * -seconds));

		if(wld->turbo) {
			debugmanager()->debug(wld->turboFactor);
			velocity *= Decimal(wld->turboFactor);
		}

		auto forward = glm::normalize(Point4(0, 0, -1, 1));
		auto abs = glm::inverse(orient->orient) * glm::inverse(camera->orientation) * forward * WORLD_DECIMAL(velocity);

		*ownPos->pos.derivative() = Point3(abs);

		oldTime = time;
	}
} } }
