#include "common.h"
#include "PlayerController.h"
#include "EventManager.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "BoundingComponent.h"
#include "ModelComponent.h"

void PlayerController::Init() {
	const std::vector<Triangle> triangles = {
		std::make_tuple(Point3(-0.5, -0.5,  0.5), Point3( 0.5, -0.5,  0.5), Point3(-0.5,  0.5,  0.5)),
		std::make_tuple(Point3( 0.5, -0.5,  0.5), Point3( 0.5,  0.5,  0.5), Point3(-0.5,  0.5,  0.5)),
		std::make_tuple(Point3(-0.5,  0.5,  0.5), Point3( 0.5,  0.5,  0.5), Point3(-0.5,  0.5, -0.5)),
		std::make_tuple(Point3( 0.5,  0.5,  0.5), Point3( 0.5,  0.5, -0.5), Point3(-0.5,  0.5, -0.5)),
		std::make_tuple(Point3(-0.5,  0.5, -0.5), Point3( 0.5,  0.5, -0.5), Point3(-0.5, -0.5, -0.5)),
		std::make_tuple(Point3( 0.5,  0.5, -0.5), Point3( 0.5, -0.5, -0.5), Point3(-0.5, -0.5, -0.5)),
		std::make_tuple(Point3(-0.5, -0.5, -0.5), Point3( 0.5, -0.5, -0.5), Point3(-0.5, -0.5,  0.5)),
		std::make_tuple(Point3( 0.5, -0.5, -0.5), Point3( 0.5, -0.5,  0.5), Point3(-0.5, -0.5,  0.5)),
		std::make_tuple(Point3( 0.5, -0.5,  0.5), Point3( 0.5, -0.5, -0.5), Point3( 0.5,  0.5,  0.5)),
		std::make_tuple(Point3( 0.5, -0.5, -0.5), Point3( 0.5,  0.5, -0.5), Point3( 0.5,  0.5,  0.5)),
		std::make_tuple(Point3(-0.5, -0.5, -0.5), Point3(-0.5, -0.5,  0.5), Point3(-0.5,  0.5, -0.5)),
		std::make_tuple(Point3(-0.5, -0.5,  0.5), Point3(-0.5,  0.5,  0.5), Point3(-0.5,  0.5, -0.5))
	};

	object = engine->AddObject();
	engine->AddComponent<PositionComponent>(object);
	engine->AddComponent<OrientationComponent>(object);
	engine->AddComponent<BoundingComponent>(object, Point3(-0.5f), Point3(1.0f));
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

					float entryTime = 0.0f;
					while(entryTime != 1.0f) {
						Point3 normal;
						auto tickVel = curr - prev;
						std::tie(entryTime, normal) = ownBounds->box.AABBSwept(objBounds->box, std::make_tuple(prev, curr, tickVel), *objPos->position);
						if(entryTime < 1.0f) {
							//float dotProduct = glm::dot(vel, normal) * entryTime;
							vel *= entryTime;
							if(glm::abs(normal.x) > 0) {
								vel.x = -vel.x;
								ownPos->position.Get().x = ownPos->position.GetPrevious().x;
							}
							if(glm::abs(normal.y) > 0) {
								vel.y = -vel.y;
								ownPos->position.Get().y = ownPos->position.GetPrevious().y;
							}
							if(glm::abs(normal.z) > 0) {
								vel.z = -vel.z;
								ownPos->position.Get().z = ownPos->position.GetPrevious().z;
							}
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
