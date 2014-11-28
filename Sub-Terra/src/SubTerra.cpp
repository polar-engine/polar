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

	auto obj = new Object();
	obj->Add<PositionComponent>();
	obj->Add<ScaleComponent>();
	obj->Add<ModelComponent>(std::initializer_list<Triangle>{
		std::make_tuple(Point(0, 0, 0, 1), Point(1, 0, 0, 1), Point(0, 1, 0, 1))
	});
	engine.AddObject(obj);

	obj = new Object();
	obj->Add<ModelComponent>(std::initializer_list<Triangle>{
		std::make_tuple(Point(-0.5, -0.75, 0, 1), Point(0.25, -0.25, 0, 1), Point(0, -0.5, 0, 1))
	});
	engine.AddObject(obj);

	engine.Init();
	engine.systems.Get<GL32Renderer>()->Use("main");
	engine.Run();
	engine.Destroy();
}
