#include "common.h"
#include "PlayerController.h"
#include "InputManager.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"

void PlayerController::Init() {
	object = new Object();
	object->Add<PositionComponent>(Point(3, 2, 5, 1));
	object->Add<OrientationComponent>();
	InitObject();
	engine->AddObject(object);

	auto pos = object->Get<PositionComponent>();
	pos->position.Derivative(1) = Point(0, -9.8f, 0, 1);
}

void PlayerController::Update(DeltaTicks &dt, std::vector<Object *> &objects) {
	auto pos = object->Get<PositionComponent>();
	auto orient = object->Get<OrientationComponent>();

	auto rel = glm::normalize(glm::fvec4(moveLeft ? -1 : 0 + moveRight ? 1 : 0, 0, moveForward ? -1 : 0 + moveBackward ? 1 : 0, 1));
	auto abs = (glm::inverse(orient->orientation) * rel) * 10.0f;
	pos->position.Derivative()->x = abs.x;
	pos->position.Derivative()->z = abs.z;
}
