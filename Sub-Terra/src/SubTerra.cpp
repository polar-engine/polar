#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "InputManager.h"
#include "GL32Renderer.h"
#include "World.h"
#include "components.h"
#include "Chunk.h"

void SubTerra::Run(const std::vector<std::string> &args) {
	Polar engine;
	engine.AddSystem<JobManager>();
	engine.AddSystem<EventManager>();
	engine.AddSystem<InputManager>();
	engine.AddSystem<AssetManager>();
	engine.AddSystem<GL32Renderer>();
	engine.AddSystem<World>(16, 16, 16);

	auto cameraObj = new Object();
	cameraObj->Add<PositionComponent>(Point(3, 2, 5, 1));
	cameraObj->Add<PlayerCameraComponent>();
	engine.AddObject(cameraObj);

	auto cameraPos = cameraObj->Get<PositionComponent>();
	auto camera = cameraObj->Get<PlayerCameraComponent>();

	float speed = 35.5f;

	auto inputM = engine.systems.Get<InputManager>();
	inputM->On(Key::Escape, [&engine] (Key) {
		engine.Quit();
	});
	inputM->When(Key::W, [cameraPos, camera, speed] (Key, const DeltaTicks &dt) {
		cameraPos->position += glm::inverse(camera->orientation) * Point(0, 0, -speed, 1) * dt.Seconds();
	});
	inputM->When(Key::S, [cameraPos, camera, speed] (Key, const DeltaTicks &dt) {
		cameraPos->position += glm::inverse(camera->orientation) * Point(0, 0, speed, 1) * dt.Seconds();
	});
	inputM->When(Key::A, [cameraPos, camera, speed] (Key, const DeltaTicks &dt) {
		cameraPos->position += glm::inverse(camera->orientation) * Point(-speed, 0, 0, 1) * dt.Seconds();
	});
	inputM->When(Key::D, [cameraPos, camera, speed] (Key, const DeltaTicks &dt) {
		cameraPos->position += glm::inverse(camera->orientation) * Point(speed, 0, 0, 1) * dt.Seconds();
	});
	inputM->When(Key::Space, [cameraPos, speed] (Key, const DeltaTicks &dt) {
		cameraPos->position.y += speed * dt.Seconds();
	});
	inputM->When(Key::C, [cameraPos, speed] (Key, const DeltaTicks &dt) {
		cameraPos->position.y -= speed * dt.Seconds();
	});
	inputM->OnMouseMove([camera] (const Point2 &delta) {
		camera->orientation = glm::quat(glm::vec3(glm::radians(0.1f) * delta.y, 0, 0)) * camera->orientation * glm::quat(glm::vec3(0, glm::radians(0.1f) * delta.x, 0));
	});

	engine.Init();
	engine.systems.Get<GL32Renderer>()->Use("main");
	engine.Run();
	engine.Destroy();
}
