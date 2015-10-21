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

	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<GL32Renderer, const std::vector<std::string> &>({"main", "perlintexture", "ssao", "cel", "fxaa", "dof"});

		engine->PushState("title");
	});

	engine.AddState("title", [] (Polar *engine, EngineState &st) {
		auto inputM = engine->GetSystem<InputManager>().lock();
		st.dtors.emplace_back(inputM->On(Key::Escape, [engine] (Key) {
			engine->Quit();
		}));
		st.dtors.emplace_back(inputM->On(Key::Space, [engine] (Key) {
			engine->PopState();
			engine->PushState("world");
		}));
	});

	engine.AddState("world", [] (Polar *engine, EngineState &st) {
		st.AddSystem<World>(16, 16, 16);
		st.AddSystem<HumanPlayerController>();

		auto inputM = engine->GetSystem<InputManager>().lock();
		st.dtors.emplace_back(inputM->On(Key::Escape, [engine] (Key) {
			engine->PopState();
			engine->PushState("title");
		}));
	});

	engine.PushState("root");
	engine.Run();
}
