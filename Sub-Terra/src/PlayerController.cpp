#include "common.h"
#include "PlayerController.h"
#include "InputManager.h"
#include "PositionComponent.h"
#include "PlayerCameraComponent.h"

void PlayerController::Init() {
	object = new Object();
	object->Add<PositionComponent>(Point(3, 2, 5, 1));
	object->Add<PlayerCameraComponent>();
	engine->AddObject(object);

	auto inputM = engine->systems.Get<InputManager>();
	auto pos = object->Get<PositionComponent>();
	auto camera = object->Get<PlayerCameraComponent>();

	float accel = 200;

	inputM->When(Key::W, [pos, camera, accel] (Key, const DeltaTicks &dt) {
		pos->position.Derivative() += glm::inverse(camera->orientation) * Point(0, 0, -accel * dt.Seconds(), 1);
	});
	inputM->When(Key::S, [pos, camera, accel] (Key, const DeltaTicks &dt) {
		pos->position.Derivative() += glm::inverse(camera->orientation) * Point(0, 0, accel * dt.Seconds(), 1);
	});
	inputM->When(Key::A, [pos, camera, accel] (Key, const DeltaTicks &dt) {
		pos->position.Derivative() += glm::inverse(camera->orientation) * Point(-accel * dt.Seconds(), 0, 0, 1);
	});
	inputM->When(Key::D, [pos, camera, accel] (Key, const DeltaTicks &dt) {
		pos->position.Derivative() += glm::inverse(camera->orientation) * Point(accel * dt.Seconds(), 0, 0, 1);
	});
	inputM->When(Key::Space, [pos, accel] (Key, const DeltaTicks &dt) {
		pos->position.Derivative() += Point(0, accel * dt.Seconds(), 0, 1);
	});
	inputM->When(Key::C, [pos, accel] (Key, const DeltaTicks &dt) {
		pos->position.Derivative() += Point(0, -accel * dt.Seconds(), 0, 1);
	});
	inputM->OnMouseMove([camera] (const Point2 &delta) {
		camera->orientation = glm::quat(glm::vec3(glm::radians(0.1f) * delta.y, 0, 0)) * camera->orientation * glm::quat(glm::vec3(0, glm::radians(0.1f) * delta.x, 0));
	});
}

void PlayerController::Update(DeltaTicks &dt, std::vector<Object *> &objects) {
	auto pos = object->Get<PositionComponent>();
	pos->position.Derivative() *= 1 - 2 * dt.Seconds();
}
