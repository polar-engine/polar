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
	engine->systems.Get<EventManager>()->ListenFor("integrator", "ticked", [this, ownPos, ownBounds] (Arg) {
		auto pair = engine->objects.right.equal_range(&typeid(BoundingComponent));
		for(auto it = pair.first; it != pair.second; ++it) {
			auto id = it->get_left();
			if(id == object) { continue; }

			auto objPos = engine->GetComponent<PositionComponent>(id);
			if(objPos != nullptr) {
				auto objBounds = engine->GetComponent<BoundingComponent>(id);
				if(objBounds != nullptr) {
					auto &prev = ownPos->position.GetPrevious();
					auto &curr = *ownPos->position;
					auto &vel = *ownPos->position.Derivative();

					/* loop until all collisions have been resolved */
					Point3 entry(0.0f);
					while(entry.x != 1.0f && entry.y != 1.0f && entry.z != 1.0f) {
						Point3 normal;
						auto tickVel = curr - prev;

						/* right now y-axis collision is done first to allow the player to walk without x/z -colliding
						 * this is somewhat hacky and could potentially be solved by doing in order of soonest to latest entry
						 */

						/* y-axis collision */
						std::tie(entry.y, normal) = ownBounds->box.AABBSwept(objBounds->box, std::make_tuple(prev, curr, Point3(0.0f, tickVel.y, 0.0f)), *objPos->position);
						if(entry.y < 1.0f) {
							float remainingTime = 1.0f - entry.y;
							curr.y = prev.y;
							vel.y *= -remainingTime * 0.5f;
						}

						/* x-axis collision */
						std::tie(entry.x, normal) = ownBounds->box.AABBSwept(objBounds->box, std::make_tuple(prev, curr, Point3(tickVel.x, 0.0f, 0.0f)), *objPos->position);
						if(entry.x < 1.0f) {
							float remainingTime = 1.0f - entry.x;
							curr.x = prev.x;
							vel.x *= -remainingTime * 0.0f;
						}

						/* z-axis collision */
						std::tie(entry.z, normal) = ownBounds->box.AABBSwept(objBounds->box, std::make_tuple(prev, curr, Point3(0.0f, 0.0f, tickVel.z)), *objPos->position);
						if(entry.z < 1.0f) {
							float remainingTime = 1.0f - entry.z;
							curr.z = prev.z;
							vel.z *= -remainingTime * 0.0f;
						}
					}
				}
			}
		}
	});
}

void PlayerController::Update(DeltaTicks &dt) {
	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto ownOrient = engine->GetComponent<OrientationComponent>(object);

	auto rel = glm::normalize(Point4((moveLeft ? -1 : 0) + (moveRight ? 1 : 0), 0, (moveForward ? -1 : 0) + (moveBackward ? 1 : 0), 1));
	auto abs = (glm::inverse(ownOrient->orientation) * rel) * 8.0f;
	ownPos->position.Derivative()->x = abs.x;
	//pos->position.Derivative()->y = abs.y;
	ownPos->position.Derivative()->z = abs.z;
}
