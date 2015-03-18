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
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

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
		orientVel.y += glm::radians(0.02f) * delta.x;
		orientVel.x += glm::radians(0.02f) * delta.y;
	});

	/* reverse camera view */

	inputM->On(Key::Z, [this, camera] (Key) {
		rearView = true;
		camera->distance = Point3(0.0f, 0.0f, 4.0f);
	});

	inputM->After(Key::Z, [this, camera] (Key) {
		rearView = false;
		camera->distance = Point3(0.0f, 0.0f, 0.0f);
	});
}

void HumanPlayerController::Update(DeltaTicks &dt) {
	PlayerController::Update(dt);

	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orientRot += orientVel;
	orientVel *= 1 - 12 * dt.Seconds();

	/* scale to range of -360 to 360 degrees */
	const auto r360 = glm::radians(360.0f);
	if(orientRot.x >  r360) { orientRot.x -= r360; }
	if(orientRot.x < -r360) { orientRot.x += r360; }
	if(orientRot.y >  r360) { orientRot.y -= r360; }
	if(orientRot.y < -r360) { orientRot.y += r360; }

	/* clamp x to range of -70 to 70 degrees */
	const auto r70 = glm::radians(70.0f);
	if(orientRot.x >  r70) { orientRot.x =  r70; }
	if(orientRot.x < -r70) { orientRot.x = -r70; }

	const auto r180 = glm::radians(180.0f);
	orient->orientation = glm::quat(Point3(0.0f, orientRot.y, 0.0f));
	camera->orientation = glm::quat(Point3(orientRot.x, rearView ? r180 : 0.0f, 0.0f));
}
