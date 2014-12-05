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
	//cameraObj->Add<PlayerCameraComponent>(Point(0, 0, 5, 1), Point(0, 0, 0, 1), Point(glm::radians(30.0f), 0, 0, 1));
	cameraObj->Add<PlayerCameraComponent>();
	cameraObj->Add<ModelComponent>(cubeVertices);
	engine.AddObject(cameraObj);

	auto cameraPos = cameraObj->Get<PositionComponent>();
	auto camera = cameraObj->Get<PlayerCameraComponent>();

	float speed = 3.5f;

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
