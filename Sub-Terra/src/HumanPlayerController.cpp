#include "common.h"
#include "HumanPlayerController.h"
#include "InputManager.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"

void HumanPlayerController::InitObject() {
	PlayerController::InitObject();
	engine->AddComponent<PlayerCameraComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);
	/* player model is 1.7f in height ranging from -0.85f to 0.85f
	 * and 0.75f in width, ranging from -0.375f to 0.375f
	 * so to position the camera in the middle of the model's face
	 * we offset the camera on the y-axis by (0.85f - 0.375f)
	 */
	camera->position = Point3(0.0f, 0.85f - 0.375f, 0.0f);
	//camera->distance = Point3(0.0f, 0.0f, 4.0f);
}

void HumanPlayerController::Init() {
	PlayerController::Init();

	auto inputM = engine->systems.Get<InputManager>();
	auto pos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);

	inputM->On(Key::W, [this] (Key) { moveForward = true; });
	inputM->On(Key::S, [this] (Key) { moveBackward = true; });
	inputM->On(Key::A, [this] (Key) { moveLeft = true; });
	inputM->On(Key::D, [this] (Key) { moveRight = true; });
	inputM->After(Key::W, [this] (Key) { moveForward = false; });
	inputM->After(Key::S, [this] (Key) { moveBackward = false; });
	inputM->After(Key::A, [this] (Key) { moveLeft = false; });
	inputM->After(Key::D, [this] (Key) { moveRight = false; });

	inputM->On(Key::Space, [pos] (Key) {
		pos->position.Derivative()->y = 9.8f / 2;
	});

	inputM->OnMouseMove([this] (const Point2 &delta) {
		orientVel.y += glm::radians(0.005f) * delta.x;
		orientVel.x += glm::radians(0.005f) * delta.y;
	});
}

void HumanPlayerController::Update(DeltaTicks &dt) {
	PlayerController::Update(dt);

	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orient->orientation = orient->orientation * glm::quat(glm::vec3(0, orientVel.y, 0));
	//orient->orientation = glm::quat(glm::vec3(orientVel.x, 0, 0)) * orient->orientation;
	camera->orientation = glm::quat(glm::vec3(orientVel.x, 0, 0)) * camera->orientation;
	orientVel *= 1 - 2 * dt.Seconds();
}
