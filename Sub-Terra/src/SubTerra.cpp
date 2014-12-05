#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "InputManager.h"
#include "GL32Renderer.h"
#include "components.h"

void SubTerra::Run(const std::vector<const std::string> &args) {
	Polar engine;
	engine.AddSystem<JobManager>();
	engine.AddSystem<EventManager>();
	engine.AddSystem<InputManager>();
	engine.AddSystem<AssetManager>();
	engine.AddSystem<GL32Renderer>();

	std::vector<Triangle> cubeVertices = {
		std::make_tuple(Point(-0.5, -0.5,  0.5, 1), Point( 0.5, -0.5,  0.5, 1), Point(-0.5,  0.5,  0.5, 1)),
		std::make_tuple(Point( 0.5, -0.5,  0.5, 1), Point( 0.5,  0.5,  0.5, 1), Point(-0.5,  0.5,  0.5, 1)),
		std::make_tuple(Point(-0.5,  0.5,  0.5, 1), Point( 0.5,  0.5,  0.5, 1), Point(-0.5,  0.5, -0.5, 1)),
		std::make_tuple(Point( 0.5,  0.5,  0.5, 1), Point( 0.5,  0.5, -0.5, 1), Point(-0.5,  0.5, -0.5, 1)),
		std::make_tuple(Point(-0.5,  0.5, -0.5, 1), Point( 0.5,  0.5, -0.5, 1), Point(-0.5, -0.5, -0.5, 1)),
		std::make_tuple(Point( 0.5,  0.5, -0.5, 1), Point( 0.5, -0.5, -0.5, 1), Point(-0.5, -0.5, -0.5, 1)),
		std::make_tuple(Point(-0.5, -0.5, -0.5, 1), Point( 0.5, -0.5, -0.5, 1), Point(-0.5, -0.5,  0.5, 1)),
		std::make_tuple(Point( 0.5, -0.5, -0.5, 1), Point( 0.5, -0.5,  0.5, 1), Point(-0.5, -0.5,  0.5, 1)),
		std::make_tuple(Point( 0.5, -0.5,  0.5, 1), Point( 0.5, -0.5, -0.5, 1), Point( 0.5,  0.5,  0.5, 1)),
		std::make_tuple(Point( 0.5, -0.5, -0.5, 1), Point( 0.5,  0.5, -0.5, 1), Point( 0.5,  0.5,  0.5, 1)),
		std::make_tuple(Point(-0.5, -0.5, -0.5, 1), Point(-0.5, -0.5,  0.5, 1), Point(-0.5,  0.5, -0.5, 1)),
		std::make_tuple(Point(-0.5, -0.5,  0.5, 1), Point(-0.5,  0.5,  0.5, 1), Point(-0.5,  0.5, -0.5, 1)),
	};

	for(int x = 0; x < 10; ++x) {
		for(int y = 0; y < 10; ++y) {
			for(int z = 0; z < 1; ++z) {
				auto obj = new Object();
				obj->Add<PositionComponent>(Point(x, y, -z, 1));
				obj->Add<OrientationComponent>();
				obj->Add<ModelComponent>(cubeVertices);
				engine.AddObject(obj);
			}
		}
	}

	auto triangleObj = new Object();
	triangleObj->Add<PositionComponent>(Point(-0.75, 0, 0, 1));
	triangleObj->Add<ModelComponent>(std::initializer_list<Triangle>{
		std::make_tuple(Point(-0.5, -0.5, 0, 1), Point(0.5, -0.5, 0, 1), Point(-0.5, 0.5, 0, 1))
	});
	engine.AddObject(triangleObj);

	auto cameraObj = new Object();
	cameraObj->Add<PositionComponent>(Point(3, 2, 5, 1));
	cameraObj->Add<PlayerCameraComponent>(Point(0, 0, 5, 1), Point(0, 0, 0, 1), Point(glm::radians(30.0f), 0, 0, 1));
	cameraObj->Add<ModelComponent>(cubeVertices);
	engine.AddObject(cameraObj);

	auto camera = cameraObj->Get<PlayerCameraComponent>();

	auto inputM = engine.systems.Get<InputManager>();
	inputM->On(Key::Escape, [&engine] (Key) {
		engine.Quit();
	});
	inputM->When(Key::W, [camera] (Key, DeltaTicks &dt) {
		camera->orientation = glm::quat(glm::vec3(glm::radians(45.0f) * dt.Seconds(), 0, 0)) * camera->orientation;
	});
	inputM->When(Key::S, [camera] (Key, DeltaTicks &dt) {
		camera->orientation = glm::quat(glm::vec3(glm::radians(-45.0f) * dt.Seconds(), 0, 0)) * camera->orientation;
	});
	inputM->When(Key::A, [camera] (Key, DeltaTicks &dt) {
		camera->orientation *= glm::quat(glm::vec3(0, glm::radians(45.0f) * dt.Seconds(), 0));
	});
	inputM->When(Key::D, [camera] (Key, DeltaTicks &dt) {
		camera->orientation *= glm::quat(glm::vec3(0, glm::radians(-45.0f) * dt.Seconds(), 0));
	});

	engine.Init();
	engine.systems.Get<GL32Renderer>()->Use("main");
	engine.Run();
	engine.Destroy();
}
