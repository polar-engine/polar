#include "SubTerra.h"
#include "Polar.h"
#include "NoRenderer.h"
#include "StdOutComponent.h"
#include "DrawableComponent.h"

void SubTerra::Run(const std::vector<const std::string> &args) {
	Polar engine;
	engine.AddSystem<NoRenderer>();
	engine.AddObject({
		new StdOutComponent("hello world"),
		new DrawableComponent()
	});
	engine.Run();
}
