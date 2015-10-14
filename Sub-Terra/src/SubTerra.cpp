#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "JobManager.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Integrator.h"
#include "GL32Renderer.h"
#include "World.h"
#include "HumanPlayerController.h"

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

	auto inputM = engine.systems.Get<InputManager>().lock();
	auto dtorEscape = inputM->On(Key::Escape, [&engine] (Key) {
		engine.Quit();
	});

	/*engine.AddState("root", [] (EngineState &st) {
		st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<GL32Renderer>();

		auto inputM = st.GetSystem<InputManager>().lock();
		st.dtors.emplace_back(inputM->On(Key::Space), [] (Key) { st.RunState("world"); });
	});

	engine.AddState("world", [] (EngineState &st) {
		st.AddSystem<World>(16, 16, 16);
		st.AddSystem<HumanPlayerController>();

		auto inputM = st.GetSystem<InputManager>().lock();
		st.dtors.emplace_back(inputM->On(Key::Escape), [] (Key) { st.QuitAll(); });
	});

	engine.RunState("root");*/

	engine.Init();
	engine.systems.Get<GL32Renderer>().lock()->MakePipeline({"main", "perlintexture", "ssao", "cel", "fxaa", "dof", "crosshairs"});
	engine.Run();
}
