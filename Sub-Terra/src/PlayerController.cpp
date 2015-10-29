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

	dtors.emplace_back(engine->AddObject(&object));
	engine->AddComponent<PositionComponent>(object);
	engine->AddComponent<OrientationComponent>(object);
	engine->AddComponent<BoundingComponent>(object, Point3(-size, -size, -size), Point3(size, size, size));
	engine->AddComponent<ModelComponent>(object, triangles);
}

void PlayerController::Update(DeltaTicks &dt) {

}
