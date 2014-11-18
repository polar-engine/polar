#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "NoRenderer.h"
#include "GL32Renderer.h"
#include "components.h"

void SubTerra::Run(const std::vector<const std::string> &args) {
	Polar engine;
	engine.AddSystem<GL32Renderer>("No supported renderers");
	auto obj = engine.AddObject();
	obj->AddComponent<PositionComponent>();
	obj->AddComponent<ScaleComponent>();
	obj->AddComponent<ModelComponent>(std::initializer_list<Triangle>{
		std::make_tuple(Point(0, 0, 0, 1), Point(1, 0, 0, 1), Point(0, 1, 0, 1))
	});
	engine.Run();
}
