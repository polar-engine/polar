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
#include "TitlePlayerController.h"
#include "HumanPlayerController.h"

void SubTerra::Run(const std::vector<std::string> &args) {
	Polar engine;


	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<AudioManager>();
		st.AddSystem<GL32Renderer, const std::vector<std::string> &>({"main", "perlintexture", "ssao", "cel", "fxaa", "gaussian"});

		engine->PushState("title");
	});

	engine.AddState("title", [] (Polar *engine, EngineState &st) {
		st.AddSystem<World>(Point3(0.5f), 16, 16, 16);
		st.AddSystem<TitlePlayerController>();

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto renderer = engine->GetSystem<GL32Renderer>().lock();

		st.dtors.emplace_back(inputM->On(Key::Escape, [engine] (Key) {
			engine->Quit();
		}));

		st.dtors.emplace_back(inputM->On(Key::Space, [engine] (Key) {
			engine->PopState();
			engine->PushState("world");
		}));

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("beep1"));

		renderer->SetUniform("u_blur", 0.05f);
	});


	engine.AddState("world", [] (Polar *engine, EngineState &st) {
		st.AddSystem<World>(Point3(0.5f), 16, 16, 16);
		st.AddSystem<HumanPlayerController>();

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto renderer = engine->GetSystem<GL32Renderer>().lock();

		st.dtors.emplace_back(inputM->On(Key::Escape, [engine] (Key) {
			engine->PopState();
			engine->PushState("title");
		}));

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("beep1"));

		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("nexus"), LoopIn{3565397});

		renderer->SetUniform("u_blur", 0.0f);
	});

	engine.PushState("root");
	engine.Run();
}
