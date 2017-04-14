
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
	TIMEDSOUND( 3.0f, "30");
	TIMEDSOUND( 3.65f, "seconds");
	TIMEDSOUND( 6.0f, "60");
	TIMEDSOUND( 6.9f, "seconds");
	TIMEDSOUND( 10.0f, "1");
	TIMEDSOUND( 10.7f, "hundred");
	TIMEDSOUND( 15.0f, "1");
	TIMEDSOUND( 15.7f, "fifty");
	TIMEDSOUND( 20.0f, "2");
	TIMEDSOUND( 20.7f, "hundred");
	TIMEDSOUND( 25.0f, "2");
	TIMEDSOUND( 25.7f, "fifty");
	TIMEDSOUND( 30.0f, "3");
	TIMEDSOUND( 30.7f, "hundred");
	TIMEDSOUND( 35.0f, "3");
	TIMEDSOUND( 35.7f, "fifty");
	TIMEDSOUND( 40.0f, "4");
	TIMEDSOUND( 40.85f, "hundred");
	TIMEDSOUND( 45.0f, "4");
	TIMEDSOUND( 45.85f, "fifty");
	TIMEDSOUND( 50.0f, "5");
	TIMEDSOUND( 50.85f, "hundred");
	TIMEDSOUND( 55.0f, "5");
	TIMEDSOUND( 55.85f, "fifty");
	TIMEDSOUND( 60.0f, "6");
	TIMEDSOUND( 60.9f, "hundred");
	TIMEDSOUND( 65.0f, "6");
	TIMEDSOUND( 65.9f, "fifty");
	TIMEDSOUND( 70.0f, "7");
	TIMEDSOUND( 70.85f, "hundred");
	TIMEDSOUND( 75.0f, "7");
	TIMEDSOUND( 75.85f, "fifty");
	TIMEDSOUND( 80.0f, "8");
	TIMEDSOUND( 80.6f, "hundred");
	TIMEDSOUND( 85.0f, "8");
	TIMEDSOUND( 85.6f, "fifty");
	TIMEDSOUND( 90.0f, "9");
	TIMEDSOUND( 90.85f, "hundred");
	TIMEDSOUND( 95.0f, "9");
	TIMEDSOUND( 95.85f, "fifty");
	TIMEDSOUND(100.0f, "freefall");

	auto font = assetM->Get<FontAsset>("nasalization-rg");
	timeDtor = engine->AddObject(&timeID);

	std::ostringstream oss;
	oss << std::setiosflags(std::ios::fixed) << std::setprecision(1) << time << 's';

	Point3 color;
	if(time > 60.0f) {
		color = Point3(1.0f, 0.75f, 0.5f);
	} else {
		color = Point3(0.7f, 0.95f, 1.0f);
	}
	float alpha = glm::mix(0.8f, 0.35f, 1.0f - glm::abs(glm::pow(glm::sin((time - 0.5f) * glm::pi<float>()), 8.0f)));

	engine->AddComponent<Text>(timeID, font, oss.str(), Point2(20, 20), Origin::TopRight, Point4(color, alpha));

	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orientVel *= static_cast<float>(glm::pow(0.995, dt.Seconds() * 1000.0));
	orient->orientation = glm::quat(Point3(orientVel.x, 0.0f, 0.0f)) * glm::quat(Point3(0.0f, orientVel.y, 0.0f)) * orient->orientation;

	const float a = 1.32499f;
	const float r = 1.01146f;
	const float k = 1.66377f;
	accum += dt.Seconds();
	velocity = 10.0f + 40.0f * a * (1.0f - glm::pow(r, k * -static_cast<float>(accum)));

	auto forward = glm::normalize(Point4(0, 0, -1, 1));
	//const float moveSpeed = 10.0f;
	//auto forward = glm::normalize(Point4(moveRight - moveLeft, 0, moveBackward - moveForward, 1)) * moveSpeed;
	auto abs = glm::inverse(orient->orientation) * glm::inverse(camera->orientation) * forward *velocity;

	*ownPos->position.Derivative() = Point3(abs);
}
