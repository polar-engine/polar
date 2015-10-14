#include "common.h"
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
	PlayerController::Init();

	engine->AddComponent<PlayerCameraComponent>(object);

	auto inputM = engine->systems.Get<InputManager>().lock();
	auto pos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	camera->position = Point3(0.0f);
	//moveForward = true;

	/* movement */
	dtors.emplace_back(inputM->On(Key::W, [this] (Key) { moveForward = true; }));
	//dtors.emplace_back(inputM->On(Key::S, [this] (Key) { moveBackward = true; }));
	//dtors.emplace_back(inputM->On(Key::A, [this] (Key) { moveLeft = true; }));
	//dtors.emplace_back(inputM->On(Key::D, [this] (Key) { moveRight = true; }));
	//dtors.emplace_back(inputM->After(Key::W, [this] (Key) { moveForward = false; }));
	//dtors.emplace_back(inputM->After(Key::S, [this] (Key) { moveBackward = false; }));
	//dtors.emplace_back(inputM->After(Key::A, [this] (Key) { moveLeft = false; }));
	//dtors.emplace_back(inputM->After(Key::D, [this] (Key) { moveRight = false; }));

	/* mouse look */
	const float mouseSpeed = 0.01f;
	dtors.emplace_back(inputM->OnMouseMove([this, mouseSpeed] (const Point2 &delta) {
		orientVel.y += glm::radians(mouseSpeed) * delta.x;
		orientVel.x += glm::radians(mouseSpeed) * delta.y;
	}));
}

void HumanPlayerController::Update(DeltaTicks &dt) {
	PlayerController::Update(dt);

	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orientRot += orientVel;
	orientVel *= 1 - 5 * dt.Seconds();

	/* scale to range of -360 to 360 degrees */
	const float r360 = glm::radians(360.0f);
	if(orientRot.x >  r360) { orientRot.x -= r360; }
	if(orientRot.x < -r360) { orientRot.x += r360; }
	if(orientRot.y >  r360) { orientRot.y -= r360; }
	if(orientRot.y < -r360) { orientRot.y += r360; }

	/* clamp x to range of -viewingAngle to viewingAngle */
	const float viewingAngle = glm::radians(90.0f);
	//if(orientRot.x >  viewingAngle) { orientRot.x = viewingAngle; }
	//if(orientRot.x < -viewingAngle) { orientRot.x = -viewingAngle; }

	const float r180 = glm::radians(180.0f);
	orient->orientation = glm::quat(Point3(orientVel.x, 0.0f, 0.0f)) * glm::quat(Point3(0.0f, orientVel.y, 0.0f)) * orient->orientation;

	static long double accum = 0.0;
	static float velocity = 10.0f;
	const float a = 1.32499f;
	const float r = 1.01146f;
	const float k = 1.66377f;
	accum += dt.Seconds();
	velocity = 10.0f + 40.0f * a * (1.0f - glm::pow(r, k * -static_cast<float>(accum)));

	auto rel = glm::normalize(Point4((moveLeft ? -1 : 0) + (moveRight ? 1 : 0), 0, (moveForward ? -1 : 0) + (moveBackward ? 1 : 0), 1));
	auto abs = (glm::inverse(orient->orientation) * glm::inverse(camera->orientation) * rel) * velocity;

	ownPos->position.Derivative()->x = abs.x;
	ownPos->position.Derivative()->y = abs.y;
	ownPos->position.Derivative()->z = abs.z;
}
