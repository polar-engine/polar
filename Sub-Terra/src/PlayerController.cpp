#include "common.h"
#include "PlayerController.h"
#include "EventManager.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "BoundingComponent.h"
#include "ModelComponent.h"
#include "PhysicalComponent.h"

void PlayerController::Init() {
	const float size = 0.1f;
	const ModelComponent::TrianglesType triangles = {
		std::make_tuple(Point3(-size, -size,  size), Point3( size, -size,  size), Point3(-size,  size,  size)),
		std::make_tuple(Point3( size, -size,  size), Point3( size,  size,  size), Point3(-size,  size,  size)),
		std::make_tuple(Point3(-size,  size,  size), Point3( size,  size,  size), Point3(-size,  size, -size)),
		std::make_tuple(Point3( size,  size,  size), Point3( size,  size, -size), Point3(-size,  size, -size)),
		std::make_tuple(Point3(-size,  size, -size), Point3( size,  size, -size), Point3(-size, -size, -size)),
		std::make_tuple(Point3( size,  size, -size), Point3( size, -size, -size), Point3(-size, -size, -size)),
		std::make_tuple(Point3(-size, -size, -size), Point3( size, -size, -size), Point3(-size, -size,  size)),
		std::make_tuple(Point3( size, -size, -size), Point3( size, -size,  size), Point3(-size, -size,  size)),
		std::make_tuple(Point3( size, -size,  size), Point3( size, -size, -size), Point3( size,  size,  size)),
		std::make_tuple(Point3( size, -size, -size), Point3( size,  size, -size), Point3( size,  size,  size)),
		std::make_tuple(Point3(-size, -size, -size), Point3(-size, -size,  size), Point3(-size,  size, -size)),
		std::make_tuple(Point3(-size, -size,  size), Point3(-size,  size,  size), Point3(-size,  size, -size))
	};

	object = engine->AddObject();
	engine->AddComponent<PositionComponent>(object);
	engine->AddComponent<OrientationComponent>(object);
	engine->AddComponent<BoundingComponent>(object, Point3(-size, -size, -size), Point3(size, size, size));
	engine->AddComponent<ModelComponent>(object, triangles);

	/* pickaxes */
	for(auto &item : hotbar) {
		item = engine->AddObject();
	}
	engine->AddComponent<PhysicalComponent>(hotbar[0], 5.0f, 25.0f, 0.88f, 200.0f);
	engine->AddComponent<PhysicalComponent>(hotbar[1], 5.0f, 80.0f, 0.88f, 200.0f);
	engine->AddComponent<PhysicalComponent>(hotbar[2], 5.0f, 400.0f, 0.95f, 200.0f);
	engine->AddComponent<PhysicalComponent>(hotbar[4], 5.0f, 25.0f, 0.28f, 200.0f);
	engine->AddComponent<PhysicalComponent>(hotbar[8], 5.0f, 1.0f, 1.0f, 200.0f);

	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto ownBounds = engine->GetComponent<BoundingComponent>(object);

	/* gravity */
	//ownPos->position.Derivative(1) = Point3(0, -9.8f, 0);

	/* collision detection and response */
	engine->systems.Get<EventManager>().lock()->ListenFor("integrator", "ticked", [this, ownPos, ownBounds] (Arg delta) {
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
						auto r = objBounds->box.AABBSwept(ownBounds->box, objPos->position.Get(), std::make_tuple(prev, curr, tickVel));
						if(std::get<0>(r) < entryTime) { std::tie(entryTime, normal) = r; }
					}
				}
			}

			if(entryTime < 1.0f) {
				engine->Quit();
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

}
