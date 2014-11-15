#include "stdafx.h"
#include "SubTerra.h"
#include "Polar.h"
#include "GL32Renderer.h"
#include "StdOutComponent.h"
#include "DrawableComponent.h"

void SubTerra::Run(const std::vector<const std::string> &args) {
	Polar engine;
	engine.AddSystem<GL32Renderer>();
	engine.AddObject({
		new StdOutComponent("hello world"),
		new DrawableComponent()
	});
	engine.Run();
}
