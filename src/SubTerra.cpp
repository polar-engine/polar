#include "stdafx.h"
#include "SubTerra.h"
#include "Polar.h"
#include "NoRenderer.h"
#include "GL32Renderer.h"
#include "StdOutComponent.h"
#include "ModelComponent.h"

void SubTerra::Run(const std::vector<const std::string> &args) {
	Polar engine;
	engine.AddSystem<GL32Renderer>("No supported renderers");
	engine.AddObject({
		new StdOutComponent("hello world"),
		new ModelComponent()
	});
	engine.Run();
}
