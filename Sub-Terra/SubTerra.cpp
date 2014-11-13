#include "SubTerra.h"
#include "Polar.h"
#include "NoRenderer.h"
#include "StdOutComponent.h"
#include "DrawableComponent.h"

void SubTerra::Run(std::vector<std::string const> const &args) {
	Polar engine;
	engine.AddSystem<NoRenderer>();
	engine.AddObject({
		new StdOutComponent("hello world"),
		new DrawableComponent()
	});
	engine.Run();
}
