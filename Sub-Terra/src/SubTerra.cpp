#include "common.h"
#include "SubTerra.h"
#include "Polar.h"
#include "JobManager.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Integrator.h"
#include "Tweener.h"
#include "AudioManager.h"
#include "GL32Renderer.h"
#include "World.h"
#include "TitlePlayerController.h"
#include "HumanPlayerController.h"

void SubTerra::Run(const std::vector<std::string> &args) {
	Polar engine;
	IDType playerID;

	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("world")});

		st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<Tweener<float>>();
		st.AddSystem<AudioManager>();
		st.AddSystem<GL32Renderer, const std::vector<std::string> &>({"main", "ssao", "cel", "fxaa", "gaussian"});

		auto assetM = engine->GetSystem<AssetManager>().lock();
		assetM->Get<AudioAsset>("beep1");
		assetM->Get<AudioAsset>("laser");
		assetM->Get<AudioAsset>("nexus");

		engine->transition = "forward";
	});

	engine.AddState("world", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("title")});

		st.AddSystem<World>(Point3(0.5f), 16, 16, 16);

		const float size = 0.05f; /* zNear */
		st.dtors.emplace_back(engine->AddObject(&playerID));
		engine->AddComponent<PositionComponent>(playerID);
		engine->AddComponent<OrientationComponent>(playerID);
		engine->AddComponent<BoundingComponent>(playerID, Point3(-size), Point3(size));

		engine->transition = "forward";
	});

	engine.AddState("title", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Pop(), Push("playing")});

		st.AddSystem<TitlePlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();

		st.dtors.emplace_back(inputM->On(Key::Escape, [engine] (Key) {
			engine->Quit();
		}));

		st.dtors.emplace_back(inputM->On(Key::Space, [engine] (Key) {
			engine->transition = "forward";
		}));

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("beep1"));

		IDType laserID;
		st.dtors.emplace_back(engine->AddObject(&laserID));
		engine->AddComponent<AudioSource>(laserID, assetM->Get<AudioAsset>("laser"), true);

		st.dtors.emplace_back(tweener->Tween(0.0f, 0.05f, 1.0f, false, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_blur", x);
		}));
	});

	engine.AddState("playing", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("back", Transition{Pop(), Pop(), Push("world")});

		st.AddSystem<HumanPlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		auto renderer = engine->GetSystem<GL32Renderer>().lock();

		st.dtors.emplace_back(inputM->On(Key::Escape, [engine] (Key) {
			engine->transition = "back";
		}));

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("beep1"));

		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("nexus"), LoopIn{3565397});

		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.71f, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_red", x);
		}, renderer->uniforms["u_red"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 2.47f, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_green", x);
		}, renderer->uniforms["u_green"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 1.53f, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_blue", x);
		}, renderer->uniforms["u_blue"]));

		st.dtors.emplace_back(tweener->Tween(0.05f, 0.0f, 1.0f, false, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_blur", x);
		}));
	});

	engine.Run("root");
}
