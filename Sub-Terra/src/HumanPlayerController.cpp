#include "common.h"
#include "HumanPlayerController.h"
#include "InputManager.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"

void HumanPlayerController::InitObject() {
	PlayerController::InitObject();
	object->Add<PlayerCameraComponent>();
	auto camera = object->Get<PlayerCameraComponent>();
	//camera->distance = Point3(0, 0, 4);
}

void HumanPlayerController::Init() {
	PlayerController::Init();

	auto inputM = engine->systems.Get<InputManager>();
	auto pos = object->Get<PositionComponent>();
	auto orient = object->Get<OrientationComponent>();
	auto camera = object->Get<PlayerCameraComponent>();

	inputM->On(Key::W, [this] (Key) { moveForward = true; });
	inputM->On(Key::S, [this] (Key) { moveBackward = true; });
	inputM->On(Key::A, [this] (Key) { moveLeft = true; });
	inputM->On(Key::D, [this] (Key) { moveRight = true; });
	inputM->After(Key::W, [this] (Key) { moveForward = false; });
	inputM->After(Key::S, [this] (Key) { moveBackward = false; });
	inputM->After(Key::A, [this] (Key) { moveLeft = false; });
	inputM->After(Key::D, [this] (Key) { moveRight = false; });

	inputM->After(Key::Space, [pos] (Key) {
		pos->position.Derivative()->y = 9.8f / 2;
	});

	inputM->OnMouseMove([this] (const Point2 &delta) {
		orientVel.y += glm::radians(0.005f) * delta.x;
		orientVel.x += glm::radians(0.005f) * delta.y;
	});
}

void HumanPlayerController::Update(DeltaTicks &dt) {
	PlayerController::Update(dt);
	auto orient = object->Get<OrientationComponent>();
	auto camera = object->Get<PlayerCameraComponent>();
	orient->orientation = orient->orientation * glm::quat(glm::vec3(0, orientVel.y, 0));
	orient->orientation = glm::quat(glm::vec3(orientVel.x, 0, 0)) * orient->orientation;
	//camera->orientation = glm::quat(glm::vec3(orientVel.x, 0, 0)) * camera->orientation;
	orientVel *= 1 - 2 * dt.Seconds();
}
