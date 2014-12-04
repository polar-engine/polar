#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "GL32Renderer.h"
#include "components.h"

void SubTerra::Run(const std::vector<const std::string> &args) {
	Polar engine;
	engine.AddSystem<JobManager>();
	engine.AddSystem<EventManager>();
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

	auto obj = new Object();
	obj->Add<PositionComponent>(Point(-0.75, 0, 0, 1));
	obj->Add<ModelComponent>(std::initializer_list<Triangle>{
		std::make_tuple(Point(-0.5, -0.5, 0, 1), Point(0.5, -0.5, 0, 1), Point(-0.5, 0.5, 0, 1))
	});
	engine.AddObject(obj);

	auto camera = new Object();
	camera->Add<PositionComponent>(Point(3, 2, 5, 1));
	camera->Add<PlayerCameraComponent>(Point(0, 0, 5, 1));
	camera->Add<ModelComponent>(cubeVertices);
	engine.AddObject(camera);

	engine.Init();
	engine.systems.Get<GL32Renderer>()->Use("main");
	engine.Run();
	engine.Destroy();
}
