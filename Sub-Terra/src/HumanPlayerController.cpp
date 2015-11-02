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
	engine->AddComponent<PlayerCameraComponent>(object);

	auto inputM = engine->GetSystem<InputManager>().lock();
	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
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

	/* collision detection and response */
	dtors.emplace_back(engine->GetSystem<EventManager>().lock()->ListenFor("integrator", "ticked", [this, ownPos, ownBounds] (Arg delta) {
		auto &prev = ownPos->position.GetPrevious();
		auto &curr = *ownPos->position;
		auto &vel = *ownPos->position.Derivative();

		Point3 normal;

		auto pair = engine->objects.right.equal_range(&typeid(BoundingComponent));

		auto tickVel = vel * delta.float_;
		float entryTime = 1.0f;

		/* find collision with soonest entry time */
		for(auto it = pair.first; it != pair.second; ++it) {
			auto id = it->get_left();

			/* don't check for collisions with self */
			if(id == object) { continue; }

			auto objPos = engine->GetComponent<PositionComponent>(id);
			if(objPos != nullptr) {
				auto objBounds = engine->GetComponent<BoundingComponent>(id);
				if(objBounds != nullptr) {
					auto r = objBounds->box.AABBSwept(ownBounds->box, objPos->position.Get(), std::make_tuple(prev, curr, tickVel));
					if(std::get<0>(r) < entryTime) { std::tie(entryTime, normal) = r; }
				}
			}
		}

		if(entryTime < 1.0f) {
			/* project velocity onto surface of collider (impulse)
			* y is given a bounce factor of 0.125
			*/
			vel -= normal * glm::dot(Point3(vel.x, vel.y * 1.125f, vel.z), normal);

			/* set current position to just before point of entry */
			curr = prev + tickVel * (entryTime - 0.0001f);

			/* set previous position to here too */
			prev += tickVel * entryTime;

			/* slide current position along surface multiplied by remaining time */
			curr += vel * delta.float_ * (1.0f - entryTime);

			engine->transition = "back";
		}
	}));
}

void HumanPlayerController::Update(DeltaTicks &dt) {
	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto orient = engine->GetComponent<OrientationComponent>(object);
	auto camera = engine->GetComponent<PlayerCameraComponent>(object);

	orientVel *= 1 - 8 * dt.Seconds();
	orient->orientation = glm::quat(Point3(orientVel.x, 0.0f, 0.0f)) * glm::quat(Point3(0.0f, orientVel.y, 0.0f)) * orient->orientation;

	const float a = 1.32499f;
	const float r = 1.01146f;
	const float k = 1.66377f;
	accum += dt.Seconds();
	velocity = 10.0f + 40.0f * a * (1.0f - glm::pow(r, k * -static_cast<float>(accum)));

	auto forward = glm::normalize(Point4(0, 0, -1, 1));
	auto abs = glm::inverse(orient->orientation) * glm::inverse(camera->orientation) * forward * velocity;

	*ownPos->position.Derivative() = Point3(abs);
}
