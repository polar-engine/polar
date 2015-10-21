#include "common.h"
#include "PlayerController.h"
#include "EventManager.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "BoundingComponent.h"
#include "ModelComponent.h"
#include "PhysicalComponent.h"

void PlayerController::Init() {
	const float size = 0.05f; /* zNear */
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

	auto ownPos = engine->GetComponent<PositionComponent>(object);
	auto ownBounds = engine->GetComponent<BoundingComponent>(object);

	/* gravity */
	//ownPos->position.Derivative(1) = Point3(0, -9.8f, 0);

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

			engine->PopState();
			engine->PushState("title");
			engine->PopState();
			engine->PushState("title");
		}
	}));
}

void PlayerController::Update(DeltaTicks &dt) {

}
