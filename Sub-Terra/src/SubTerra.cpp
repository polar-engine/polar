#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "InputManager.h"
#include "Integrator.h"
#include "GL32Renderer.h"
#include "World.h"
#include "HumanPlayerController.h"
#include "components.h"

void SubTerra::Run(const std::vector<std::string> &args) {
	Polar engine;
	engine.AddSystem<JobManager>();
	engine.AddSystem<EventManager>();
	engine.AddSystem<InputManager>();
	engine.AddSystem<AssetManager>();
	engine.AddSystem<Integrator>();
	engine.AddSystem<GL32Renderer>();
	engine.AddSystem<World>(16, 16, 16);
	engine.AddSystem<HumanPlayerController>();

	auto inputM = engine.systems.Get<InputManager>();
	inputM->On(Key::Escape, [&engine] (Key) {
		engine.Quit();
	});

	engine.Init();
	engine.systems.Get<GL32Renderer>()->MakePipeline({"main", "fog"});
	engine.Run();
	engine.Destroy();
}
