
#include "common.h"
#include <iomanip>
#include <glm/gtc/noise.hpp>
#include "HumanPlayerController.h"
#include "AssetManager.h"
#include "EventManager.h"
#include "InputManager.h"
#include "World.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"
#include "BoundingComponent.h"
#include "PhysicalComponent.h"
#include "Text.h"
#include "AudioSource.h"

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
		orientVel.y += glm::radians(mouseSpeed) * delta.x;
		orientVel.x += glm::radians(mouseSpeed) * delta.y;
	}));
	dtors.emplace_back(inputM->OnControllerAxes([this, mouseSpeed] (const Point2 &delta) {
		orientVel.y += glm::radians(mouseSpeed) * delta.x * 30.0f;
		orientVel.x += glm::radians(mouseSpeed) * delta.y * 30.0f;
	}));
	//dtors.emplace_back(inputM->On(Key::W, [this] (Key) { moveForward = true; }));
	//dtors.emplace_back(inputM->On(Key::S, [this] (Key) { moveBackward = true; }));
	//dtors.emplace_back(inputM->On(Key::A, [this] (Key) { moveLeft = true; }));
	//dtors.emplace_back(inputM->On(Key::D, [this] (Key) { moveRight = true; }));
	//dtors.emplace_back(inputM->After(Key::W, [this] (Key) { moveForward = false; }));
	//dtors.emplace_back(inputM->After(Key::S, [this] (Key) { moveBackward = false; }));
	//dtors.emplace_back(inputM->After(Key::A, [this] (Key) { moveLeft = false; }));
	//dtors.emplace_back(inputM->After(Key::D, [this] (Key) { moveRight = false; }));

	/* collision detection and response */
	dtors.emplace_back(engine->GetSystem<EventManager>().lock()->ListenFor("integrator", "ticked", [this, ownPos, ownBounds] (Arg delta) {
		auto world = engine->GetSystem<World>().lock();

		if(world) {
			auto &curr = *ownPos->position;
			if(world->Eval(curr)) {
				engine->transition = "back";
			}
		}
	}));
}

#define TIMEDSOUND(TIME, NAME)                                                     \
	if(oldTime < TIME && time >= TIME) {                                           \
		soundDtors[soundIndex++] = engine->AddObject(&soundID);                    \
		soundIndex %= soundDtors.size();                                           \
		engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>(NAME)); \
	}

void HumanPlayerController::Update(DeltaTicks &dt) {
	auto assetM = engine->GetSystem<AssetManager>().lock();

	auto oldTime = time;
	time += dt.Seconds();

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

	engine->AddComponent<Text>(timeID, font, oss.str(), Point2(20, 20), Origin::TopRight, Point4(color, alpha));

	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orientVel *= static_cast<float>(glm::pow(0.995, dt.Seconds() * 1000.0));
	orient->orientation = Quat(Point3(orientVel.x, 0.0, 0.0)) * Quat(Point3(0.0, orientVel.y, 0.0)) * orient->orientation;

	const Decimal a(1.32499);
	const Decimal r(1.01146);
	const Decimal k(1.66377);
	accum += dt.Seconds();
	velocity = 10.0 + 40.0 * a * (1.0 - glm::pow(r, k * -static_cast<Decimal>(accum)));

	auto forward = glm::normalize(Point4(0, 0, -1, 1));
	//const float moveSpeed = 10.0f;
	//auto forward = glm::normalize(Point4(moveRight - moveLeft, 0, moveBackward - moveForward, 1)) * moveSpeed;
	auto abs = glm::inverse(orient->orientation) * glm::inverse(camera->orientation) * forward * velocity;

	*ownPos->position.Derivative() = Point3(abs);
}
