#include "common.h"
#include "Freefall.h"
#include <random>
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
#include "Menu.h"
#include "TitlePlayerController.h"
#include "HumanPlayerController.h"
#include "BoundingComponent.h"
#include "Text.h"

void Freefall::Run(const std::vector<std::string> &args) {
	const double secsPerBeat = 1.2631578947368421;
	Polar engine;
	IDType playerID;

	std::mt19937_64 rng(time(0));

	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("world")});

		//st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<Tweener<float>>();
		st.AddSystem<AudioManager>();
		st.AddSystem<GL32Renderer, const boost::container::vector<std::string> &>({ "perlin" });

		auto assetM = engine->GetSystem<AssetManager>().lock();
		assetM->Get<AudioAsset>("beep1");
		assetM->Get<AudioAsset>("laser");
		assetM->Get<AudioAsset>("nexus");

		engine->transition = "forward";
	});

	engine.AddState("world", [&rng, &playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("title")});

		auto renderer = engine->GetSystem<GL32Renderer>().lock();

		float seed = (float)rng() / rng.max() * 1000.0f;
		st.AddSystem<World>(seed);
		renderer->SetUniform("u_seed", seed);

		st.dtors.emplace_back(engine->AddObject(&playerID));

		engine->AddComponent<PositionComponent>(playerID);
		engine->AddComponent<OrientationComponent>(playerID);

		engine->transition = "forward";
	});

	engine.AddState("title", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Pop(), Push("playing")});

		st.AddSystem<TitlePlayerController>(playerID);
		st.AddSystem<Menu>();

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		auto renderer = engine->GetSystem<GL32Renderer>().lock();

		st.dtors.emplace_back(inputM->On(Key::Escape,         [engine] (Key) { engine->Quit(); }));
		st.dtors.emplace_back(inputM->On(Key::ControllerBack, [engine] (Key) { engine->Quit(); }));

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

		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 1.0, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_red", x);
		}, 5.0 * 4 - 1.0, renderer->uniforms["u_red"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_green", x);
		}, 5.0 * 2 - 1.0, renderer->uniforms["u_green"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_blue", x);
		}, 5.0 - 1.0, renderer->uniforms["u_blue"]));
	});

	engine.AddState("playing", [secsPerBeat, &playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("back", Transition{Pop(), Pop(), Push("world")});

		st.AddSystem<HumanPlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		auto renderer = engine->GetSystem<GL32Renderer>().lock();

		st.dtors.emplace_back(inputM->On(Key::Escape,         [engine] (Key) { engine->transition = "back"; }));
		st.dtors.emplace_back(inputM->On(Key::ControllerBack, [engine] (Key) { engine->transition = "back"; }));

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("beep1"));

		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("nexus"), LoopIn{3565397});

		st.dtors.emplace_back(tweener->Tween(0.05f, 0.0f, 1.0f, false, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_blur", x);
		}));

		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_red", x);
		}, secsPerBeat * 4 - 0.5, renderer->uniforms["u_red"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_green", x);
		}, secsPerBeat * 2 - 0.5, renderer->uniforms["u_green"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<GL32Renderer>().lock();
			renderer->SetUniform("u_blue", x);
		}, secsPerBeat - 0.5, renderer->uniforms["u_blue"]));
	});

	engine.Run("root");
}
