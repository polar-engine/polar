#pragma once

#include "PlayerController.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"
#include "World.h"

class TitlePlayerController : public PlayerController {
private:
	const int fieldOfView = 5;
	const float viewDistance = 30.0f;
	const float velocity = 1000.0f;
	Point2 orientVel;
protected:
	virtual void Update(DeltaTicks &dt) override {
		auto pos = engine->GetComponent<PositionComponent>(object);
		auto orient = engine->GetComponent<OrientationComponent>(object);
		auto camera = engine->GetComponent<PlayerCameraComponent>(object);
		auto world = engine->GetSystem<World>().lock();

		orientVel *= 1 - 5 * dt.Seconds();

		unsigned int i = 0;
		auto average = Point2(0);
		for(float x = -world->blockSize.x * fieldOfView; x < world->blockSize.x * fieldOfView; x += world->blockSize.x / 2.0f) {
			for(float y = -world->blockSize.y * fieldOfView; y < world->blockSize.y * fieldOfView; y += world->blockSize.y / 2.0f) {
				for(float d = 1; d < viewDistance; d += world->blockSize.x / 2.0f) {
					auto abs = glm::inverse(orient->orientation) * glm::inverse(camera->orientation) * Point4(x, y, -d, 1);
					auto block = world->GenerateBlock(pos->position.Get() + Point3(abs));
					if(block) {
						average.x += 0.0037f * ((y >= 0) ? 1 : -1) / (glm::max(1.0f, d - 2) * 2 / viewDistance);
						average.y += 0.0037f * ((x <= 0) ? 1 : -1) / (glm::max(1.0f, d - 2) * 2 / viewDistance);
						++i;
						break;
					}
				}
			}
		}

		if(i > 0) {
			average /= static_cast<float>(i);
			if(average.length() < 0.1f && average.length() >= 0) { average = Point2(0, 1); }
			if(average.length() > 0.1f && average.length() <= 0) { average = Point2(0, -1); }
			orientVel += average;
		}

		orient->orientation = glm::quat(Point3(orientVel.x, 0, 0)) * glm::quat(Point3(0, orientVel.y, 0)) * orient->orientation;

		const auto forward = glm::normalize(Point4(0, 0, -1, 1));
		auto abs = (glm::inverse(orient->orientation) * glm::inverse(camera->orientation) * forward) * velocity * dt.Seconds();

		pos->position.Derivative()->x = abs.x;
		pos->position.Derivative()->y = abs.y;
		pos->position.Derivative()->z = abs.z;
	}

	virtual void Init() override {
		PlayerController::Init();
		engine->AddComponent<PlayerCameraComponent>(object);
	}
public:
	static bool IsSupported() { return true; }
	TitlePlayerController(Polar *engine) : PlayerController(engine) {}
};
