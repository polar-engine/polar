
#include <polar/common.h>
#include <iomanip>
#include <glm/gtc/noise.hpp>
#include <polar/HumanPlayerController.h>
#include <polar/AssetManager.h>
#include <polar/EventManager.h>
#include <polar/InputManager.h>
#include <polar/World.h>
#include <polar/PositionComponent.h>
#include <polar/ScreenPositionComponent.h>
#include <polar/ColorComponent.h>
#include <polar/OrientationComponent.h>
#include <polar/PlayerCameraComponent.h>
#include <polar/BoundingComponent.h>
#include <polar/PhysicalComponent.h>
#include <polar/Text.h>
#include <polar/AudioSource.h>

void HumanPlayerController::Init() {
	engine->AddComponent<PlayerCameraComponent>(object);

	auto inputM = engine->GetSystem<InputManager>().lock();
	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto ownBounds = engine->GetComponent<BoundingComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	camera->position = Point3(0.0f);

	/* mouse look */
	const float mouseSpeed = 0.015f;
	dtors.emplace_back(inputM->OnMouseMove([this, mouseSpeed] (const Point2 &delta) {
		Decimal smoothingVelFactor = glm::pow(Decimal(2) - smoothing, Decimal(40));
		orientVel.y += glm::radians(mouseSpeed) * delta.x * smoothingVelFactor;
		orientVel.x += glm::radians(mouseSpeed) * delta.y * smoothingVelFactor;
	}));
	dtors.emplace_back(inputM->OnControllerAxes([this, mouseSpeed] (const Point2 &delta) {
		Decimal smoothingVelFactor = glm::pow(Decimal(2) - smoothing, Decimal(40));
		orientVel.y += glm::radians(mouseSpeed) * delta.x * 30.0f * smoothingVelFactor;
		orientVel.x += glm::radians(mouseSpeed) * delta.y * 30.0f * smoothingVelFactor;
	}));
	dtors.emplace_back(inputM->OnAnalog("ingame_camera", [this, mouseSpeed] (const Point2 &delta) {
		Decimal smoothingVelFactor = glm::pow(Decimal(2) - smoothing, Decimal(40));
		orientVel.y += glm::radians(mouseSpeed) * delta.x * smoothingVelFactor;
		orientVel.x += glm::radians(mouseSpeed) * delta.y * smoothingVelFactor;
	}));

	dtors.emplace_back(inputM->On(Key::Space, [this] (Key) {
		engine->GetSystem<World>().lock()->turbo = true;
	}));
	dtors.emplace_back(inputM->After(Key::Space, [this] (Key) {
		engine->GetSystem<World>().lock()->turbo = false;
	}));

	/* collision detection and response */
	dtors.emplace_back(engine->GetSystem<EventManager>().lock()->ListenFor("integrator", "ticked", [this, ownPos, ownBounds] (Arg delta) {
		auto world = engine->GetSystem<World>().lock();

		if(world) {
			auto &curr = *ownPos->position;
			if(world->Eval(curr)) {
				engine->transition = "gameover";
			}
		}
	}));
}

#define TIMEDSOUND(TIME, NAME)                                                                              \
	if(oldTime < TIME && time >= TIME) {                                                                    \
		soundDtors[soundIndex++] = engine->AddObject(&soundID);                                             \
		soundIndex %= soundDtors.size();                                                                    \
		engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>(NAME), AudioSourceType::Effect); \
	}

void HumanPlayerController::Update(DeltaTicks &dt) {
	auto assetM = engine->GetSystem<AssetManager>().lock();
	auto inputM = engine->GetSystem<InputManager>().lock();
	auto world = engine->GetSystem<World>().lock();

	auto time = world->GetTicks() / Decimal(ENGINE_TICKS_PER_SECOND);

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

	auto font = assetM->Get<FontAsset>("nasalization-rg");
	timeDtor = engine->AddObject(&timeID);

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

	engine->AddComponent<Text>(timeID, font, oss.str());
	engine->AddComponent<ScreenPositionComponent>(timeID, Point2(20, 20), Origin::TopRight);
	engine->AddComponent<ColorComponent>(timeID, Point4(color, alpha));

	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orientVel *= static_cast<float>(glm::pow(smoothing, dt.Seconds() * Decimal(1000)));
	orient->orientation = Quat(Point3(orientVel.x, 0.0, 0.0)) * Quat(Point3(0.0, orientVel.y, 0.0)) * orient->orientation;

	Decimal seconds = engine->GetSystem<World>().lock()->GetTicks() / Decimal(ENGINE_TICKS_PER_SECOND);
	const Decimal a = Decimal(1.32499);
	const Decimal r = Decimal(1.01146);
	const Decimal k = Decimal(1.66377);
	velocity = 10 + 40 * a * (1 - glm::pow(r, k * -seconds));

	if(world->turbo) {
		DebugManager()->Debug(world->turboFactor);
		velocity *= Decimal(world->turboFactor);
	}

	auto forward = glm::normalize(Point4(0, 0, -1, 1));
	auto abs = glm::inverse(orient->orientation) * glm::inverse(camera->orientation) * forward * WORLD_DECIMAL(velocity);

	*ownPos->position.Derivative() = Point3(abs);

	oldTime = time;
}
