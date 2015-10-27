#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "JobManager.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Integrator.h"
#include "AudioManager.h"
#include "GL32Renderer.h"
#include "World.h"
#include "HumanPlayerController.h"

void SubTerra::Run(const std::vector<std::string> &args) {
	Polar engine;

	auto beep = engine.GetSystem<AssetManager>().lock()->Get<AudioAsset>("beep1");
	auto music = engine.GetSystem<AssetManager>().lock()->Get<AudioAsset>("nexus");

	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<AudioManager>();
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


	engine.AddState("world", [&beep, &music] (Polar *engine, EngineState &st) {
		st.AddSystem<World>(16, 16, 16);
		st.AddSystem<HumanPlayerController>();

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, beep);

		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, music, LoopIn{3565397});

		auto inputM = engine->GetSystem<InputManager>().lock();
		st.dtors.emplace_back(inputM->On(Key::Escape, [engine] (Key) {
			engine->PopState();
			engine->PushState("title");
		}));
	});

	engine.PushState("root");
	engine.Run();
}
