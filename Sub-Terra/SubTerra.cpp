#include "SubTerra.h"
#include "Polar.h"
#include "NoRenderer.h"
#include "StdOutComponent.h"

void SubTerra::Run(std::vector<std::string const> const &args) {
	Polar engine;
	engine.AddSystem<NoRenderer>();
	engine.AddObject({ new StdOutComponent("hello world") });
	engine.Run();
}
