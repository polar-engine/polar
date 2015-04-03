#include "common.h"
#include "HumanPlayerController.h"
#include "EventManager.h"
#include "InputManager.h"
#include "World.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"
#include "BoundingComponent.h"

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

	auto inputM = engine->systems.Get<InputManager>().lock();
	auto pos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	dtors.emplace_back(inputM->On(Key::W, [this] (Key) { moveForward = true; }));
	dtors.emplace_back(inputM->On(Key::S, [this] (Key) { moveBackward = true; }));
	dtors.emplace_back(inputM->On(Key::A, [this] (Key) { moveLeft = true; }));
	dtors.emplace_back(inputM->On(Key::D, [this] (Key) { moveRight = true; }));
	dtors.emplace_back(inputM->After(Key::W, [this] (Key) { moveForward = false; }));
	dtors.emplace_back(inputM->After(Key::S, [this] (Key) { moveBackward = false; }));
	dtors.emplace_back(inputM->After(Key::A, [this] (Key) { moveLeft = false; }));
	dtors.emplace_back(inputM->After(Key::D, [this] (Key) { moveRight = false; }));

	dtors.emplace_back(inputM->On(Key::Space, [pos] (Key) {
		pos->position.Derivative()->y = 9.8f / 2;
	}));

	dtors.emplace_back(inputM->OnMouseMove([this] (const Point2 &delta) {
		orientVel.y += glm::radians(0.02f) * delta.x;
		orientVel.x += glm::radians(0.02f) * delta.y;
	}));

	/* reverse camera view */

	dtors.emplace_back(inputM->On(Key::Z, [this, camera] (Key) {
		rearView = true;
		camera->distance = Point3(0.0f, 0.0f, 4.0f);
	}));

	dtors.emplace_back(inputM->After(Key::Z, [this, camera] (Key) {
		rearView = false;
		camera->distance = Point3(0.0f, 0.0f, 0.0f);
	}));

	dtors.emplace_back(inputM->On(Key::E, [this, pos, orient, camera] (Key) {
		auto origin = pos->position.Get() + camera->position.Get();
		auto direction = Point3(glm::toMat3(orient->orientation * camera->orientation) * Point3(0.0f, 0.0f, 1.0f));
		direction.z = -direction.z;

		float entryTime = std::numeric_limits<float>::infinity();
		IDType soonestId = 0;
		Point3 soonestPos = Point3(0.0f);

		auto pair = engine->objects.right.equal_range(&typeid(BoundingComponent));
		for(auto it = pair.first; it != pair.second; ++it) {
			auto id = it->get_left();

			/* don't check against self */
			if(id == object) { continue; }

			auto objPos = engine->GetComponent<PositionComponent>(id);
			if(objPos != nullptr) {
				auto objBounds = engine->GetComponent<BoundingComponent>(id);
				if(objBounds != nullptr) {
					auto r = objBounds->box.TestRay(origin, direction, 8.0f, objPos->position.Get());
					if(std::get<0>(r) && std::get<1>(r) < entryTime) {
						std::tie(std::ignore, entryTime, soonestPos) = r;
						soonestId = id;
					}
				}
			}
		}

		if(soonestId != 0) {
			auto world = engine->systems.Get<World>().lock();
			world->SetBlock(world->BlockCoordForPos(soonestPos), false);
		}
	}));
}

void HumanPlayerController::Update(DeltaTicks &dt) {
	PlayerController::Update(dt);

	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orientRot += orientVel;
	orientVel *= 1 - 12 * dt.Seconds();

	/* scale to range of -360 to 360 degrees */
	const float r360 = glm::radians(360.0f);
	if(orientRot.x >  r360) { orientRot.x -= r360; }
	if(orientRot.x < -r360) { orientRot.x += r360; }
	if(orientRot.y >  r360) { orientRot.y -= r360; }
	if(orientRot.y < -r360) { orientRot.y += r360; }

	/* clamp x to range of -viewingAngle to viewingAngle */
	const float viewingAngle = glm::radians(90.0f);
	if(orientRot.x >  viewingAngle) { orientRot.x = viewingAngle; }
	if(orientRot.x < -viewingAngle) { orientRot.x = -viewingAngle; }

	const float r180 = glm::radians(180.0f);
	orient->orientation = glm::quat(Point3(0.0f, orientRot.y, 0.0f));
	camera->orientation = glm::quat(Point3(orientRot.x, rearView ? r180 : 0.0f, 0.0f));

	/* walking head bobbing */

	const float bobInterval = 0.4f;
	const float bobHalfFreq = r180 / bobInterval;
	const float bobDropAngle = glm::radians(0.9f);
	const float bobRollAngle = glm::radians(0.2f);
	static float bobCounter = 0.0f;

	/* increment if moving */
	if(moveForward | moveBackward | moveLeft | moveRight) {
		bobCounter += dt.Seconds() * bobHalfFreq;
	}
	/* increment until center if not centered */
	else if(bobCounter != 0.0f) {
		/* calculate new counter value and scale to range of 0 to 360 degrees */
		auto newCounter = bobCounter + dt.Seconds() * bobHalfFreq;
		if(newCounter >= r360) { newCounter -= r360; }

		/* if scaled new counter is no longer greater than old counter */
		if(newCounter <= bobCounter) { newCounter = 0.0f; }
		bobCounter = newCounter;
	}

	/* scale to range of 0 to 360 degrees */
	if(bobCounter >= r360) { bobCounter -= r360; }

	float bobDrop = glm::cos(bobCounter);
	float bobRoll = glm::sin(bobCounter);

	camera->orientation = glm::quat(Point3(-glm::abs(bobDrop) * bobDropAngle, 0.0f, 0.0f)) * camera->orientation;
	camera->orientation = glm::quat(Point3(0.0f, 0.0f, glm::clamp(bobRoll * bobRollAngle * 1.25f, -bobRollAngle, bobRollAngle))) * camera->orientation;
}
