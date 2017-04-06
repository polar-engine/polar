
#include "common.h"
#include <glm/gtc/noise.hpp>
#include "HumanPlayerController.h"
#include "EventManager.h"
#include "InputManager.h"
#include "World.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"
#include "BoundingComponent.h"
#include "PhysicalComponent.h"

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
		auto &curr = *ownPos->position;

		float eval = glm::simplex(curr * 0.05f + 10.0f) * 0.5 + 0.5;
		if(eval > 0.7) {
			engine->transition = "back";
		}
	}));
}

void HumanPlayerController::Update(DeltaTicks &dt) {
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
