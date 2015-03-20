#include "common.h"
#include "PlayerController.h"
#include "EventManager.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "BoundingComponent.h"
#include "ModelComponent.h"

void PlayerController::Init() {
	const std::vector<Triangle> triangles = {
		std::make_tuple(Point3(-0.375, -0.85,  0.25), Point3( 0.375, -0.85,  0.25), Point3(-0.375,  0.85,  0.25)),
		std::make_tuple(Point3( 0.375, -0.85,  0.25), Point3( 0.375,  0.85,  0.25), Point3(-0.375,  0.85,  0.25)),
		std::make_tuple(Point3(-0.375,  0.85,  0.25), Point3( 0.375,  0.85,  0.25), Point3(-0.375,  0.85, -0.25)),
		std::make_tuple(Point3( 0.375,  0.85,  0.25), Point3( 0.375,  0.85, -0.25), Point3(-0.375,  0.85, -0.25)),
		std::make_tuple(Point3(-0.375,  0.85, -0.25), Point3( 0.375,  0.85, -0.25), Point3(-0.375, -0.85, -0.25)),
		std::make_tuple(Point3( 0.375,  0.85, -0.25), Point3( 0.375, -0.85, -0.25), Point3(-0.375, -0.85, -0.25)),
		std::make_tuple(Point3(-0.375, -0.85, -0.25), Point3( 0.375, -0.85, -0.25), Point3(-0.375, -0.85,  0.25)),
		std::make_tuple(Point3( 0.375, -0.85, -0.25), Point3( 0.375, -0.85,  0.25), Point3(-0.375, -0.85,  0.25)),
		std::make_tuple(Point3( 0.375, -0.85,  0.25), Point3( 0.375, -0.85, -0.25), Point3( 0.375,  0.85,  0.25)),
		std::make_tuple(Point3( 0.375, -0.85, -0.25), Point3( 0.375,  0.85, -0.25), Point3( 0.375,  0.85,  0.25)),
		std::make_tuple(Point3(-0.375, -0.85, -0.25), Point3(-0.375, -0.85,  0.25), Point3(-0.375,  0.85, -0.25)),
		std::make_tuple(Point3(-0.375, -0.85,  0.25), Point3(-0.375,  0.85,  0.25), Point3(-0.375,  0.85, -0.25))
	};

	object = engine->AddObject();
	engine->AddComponent<PositionComponent>(object);
	engine->AddComponent<OrientationComponent>(object);
	engine->AddComponent<BoundingComponent>(object, Point3(-0.375f, -0.85f, -0.25f), Point3(0.75f, 1.7f, 0.5f));
	engine->AddComponent<ModelComponent>(object, triangles);
	InitObject();

	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto ownBounds = engine->GetComponent<BoundingComponent>(object);

	/* gravity */
	ownPos->position.Derivative(1) = Point3(0, -9.8f, 0);

	/* collision detection and response */
	engine->systems.Get<EventManager>()->ListenFor("integrator", "ticked", [this, ownPos, ownBounds] (Arg delta) {
		auto &prev = ownPos->position.GetPrevious();
		auto &curr = *ownPos->position;
		auto &vel = *ownPos->position.Derivative();

		Point3 normal;
		float entryTime = 0.0f;

		auto pair = engine->objects.right.equal_range(&typeid(BoundingComponent));

		/* loop until all collisions have been resolved */
		while(entryTime != 1.0f) {
			auto tickVel = vel * delta.float_;
			entryTime = 1.0f;

			/* find collision with soonest entry time */
			for(auto it = pair.first; it != pair.second; ++it) {
				auto id = it->get_left();

				/* don't check for collisions with self */
				if(id == object) { continue; }

				auto objPos = engine->GetComponent<PositionComponent>(id);
				if(objPos != nullptr) {
					auto objBounds = engine->GetComponent<BoundingComponent>(id);
					if(objBounds != nullptr) {
						auto r = ownBounds->box.AABBSwept(objBounds->box, std::make_tuple(prev, curr, tickVel), *objPos->position);
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
			}
		}
	});
}

void PlayerController::Update(DeltaTicks &dt) {
	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto ownOrient = engine->GetComponent<OrientationComponent>(object);

	auto rel = glm::normalize(Point4((moveLeft ? -1 : 0) + (moveRight ? 1 : 0), 0, (moveForward ? -1 : 0) + (moveBackward ? 1 : 0), 1));
	auto abs = (glm::inverse(ownOrient->orientation) * rel) * 3.5f;

	ownPos->position.Derivative()->x = abs.x;
	//pos->position.Derivative()->y = abs.y;
	ownPos->position.Derivative()->z = abs.z;
}
