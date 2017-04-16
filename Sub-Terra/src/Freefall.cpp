#include "common.h"
#include "Freefall.h"
#include <glm/gtc/random.hpp>
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
#include "MenuSystem.h"
#include "TitlePlayerController.h"
#include "HumanPlayerController.h"
#include "BoundingComponent.h"
#include "Text.h"

void Freefall::Run(const std::vector<std::string> &args) {
	const double secsPerBeat = 1.2631578947368421;
	Polar engine;
	IDType playerID;

	srand((unsigned int)time(0));
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
		st.AddSystemAs<Renderer, GL32Renderer, const boost::container::vector<std::string> &>({ "perlin" });

		auto assetM = engine->GetSystem<AssetManager>().lock();
		assetM->Get<AudioAsset>("nexus");
		assetM->Get<AudioAsset>("laser");
		assetM->Get<AudioAsset>("beep1");
		assetM->Get<AudioAsset>("menu1");
		assetM->Get<AudioAsset>("30");
		assetM->Get<AudioAsset>("60");
		assetM->Get<AudioAsset>("1");
		assetM->Get<AudioAsset>("2");
		assetM->Get<AudioAsset>("3");
		assetM->Get<AudioAsset>("4");
		assetM->Get<AudioAsset>("5");
		assetM->Get<AudioAsset>("6");
		assetM->Get<AudioAsset>("7");
		assetM->Get<AudioAsset>("8");
		assetM->Get<AudioAsset>("9");
		assetM->Get<AudioAsset>("seconds");
		assetM->Get<AudioAsset>("hundred");
		assetM->Get<AudioAsset>("fifty");
		assetM->Get<AudioAsset>("freefall");

		engine->transition = "forward";
	});

	engine.AddState("world", [&rng, &playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("title")});

		st.AddSystem<World>();

		st.dtors.emplace_back(engine->AddObject(&playerID));

		Point3 seed = glm::ballRand(Decimal(1000.0));
		engine->AddComponent<PositionComponent>(playerID, seed);
		engine->AddComponent<OrientationComponent>(playerID);

		auto renderer = engine->GetSystem<Renderer>().lock();
		renderer->SetUniform("u_baseThreshold", 0.7);
		renderer->SetUniform("u_beatTicks",  1000.0);
		renderer->SetUniform("u_beatPower",     4.0);
		renderer->SetUniform("u_beatStrength",  0.005);
		renderer->SetUniform("u_waveTicks",  2345.0);
		renderer->SetUniform("u_wavePower",     8.0);
		renderer->SetUniform("u_waveStrength",  0.02);
		renderer->SetUniform("u_worldScale", Point3(20.0));

		engine->transition = "forward";
	});

	engine.AddState("title", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{ Pop(), Push("playing") });
		st.transitions.emplace("back", Transition{ QuitAction() });

		st.AddSystem<TitlePlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		auto renderer = engine->GetSystem<Renderer>().lock();
		auto audioM = engine->GetSystem<AudioManager>().lock();

		Menu menu = {
			MenuItem("Solo Play", [engine] (Decimal) {
				engine->transition = "forward";
				return false;
			}),
			MenuItem("Options", {
				MenuItem("Graphics", {
					MenuItem("Base Detail", MenuControl::Slider<Decimal>(6, 30, renderer->GetUniformDecimal("u_baseDetail", 10)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_baseDetail", x);
						return true;
					}),
					//MenuItem("Far Detail", MenuControl::Slider<Decimal>(), [] (Decimal) { return true; }),
					//MenuItem("Far Limiter", MenuControl::Slider<Decimal>(), [] (Decimal) { return true; }),
					//MenuItem("Precision", MenuControl::Selection({"Float", "Double"}), [] (Decimal) { return true; }),
					MenuItem("Retro Factor 1", MenuControl::Slider<Decimal>(0, 20, renderer->GetUniformDecimal("u_retroFactor", 0)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_retroFactor", x);
						return true;
					}),
					MenuItem("Retro Factor 2", MenuControl::Slider<Decimal>(0, 20, renderer->GetUniformDecimal("u_retroFactor2", 0)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_retroFactor2", x);
						return true;
					}),
					MenuItem("Show FPS", MenuControl::Checkbox(renderer->showFPS), [engine] (Decimal state) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->showFPS = state;
						return true;
					}),
				}),
				MenuItem("Audio", {
					MenuItem("Mute", MenuControl::Checkbox(audioM->muted), [engine] (Decimal state) {
						auto audioM = engine->GetSystem<AudioManager>().lock();
						audioM->muted = state;
						return true;
					}),
				}),
				//MenuItem("Controls", [] (Decimal) { return true; }),
				MenuItem("World", {
					MenuItem("u_baseThreshold", MenuControl::Slider<Decimal>(0, 1, renderer->GetUniformDecimal("u_baseThreshold"), 0.05), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_baseThreshold", x);
						return true;
					}),
					MenuItem("u_beatTicks", MenuControl::Slider<Decimal>(50, 10000, renderer->GetUniformDecimal("u_beatTicks"), 50), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_beatTicks", x);
						return true;
					}),
					MenuItem("u_beatPower", MenuControl::Slider<Decimal>(1, 16, renderer->GetUniformDecimal("u_beatPower")), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_beatPower", x);
						return true;
					}),
					MenuItem("u_beatStrength", MenuControl::Slider<Decimal>(-1, 1, renderer->GetUniformDecimal("u_beatStrength"), 0.002), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_beatStrength", x);
						return true;
					}),
					MenuItem("u_waveTicks", MenuControl::Slider<Decimal>(50, 10000, renderer->GetUniformDecimal("u_waveTicks"), 50), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_waveTicks", x);
						return true;
					}),
					MenuItem("u_wavePower", MenuControl::Slider<Decimal>(1, 16, renderer->GetUniformDecimal("u_wavePower")), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_wavePower", x);
						return true;
					}),
					MenuItem("u_waveStrength", MenuControl::Slider<Decimal>(-1, 1, renderer->GetUniformDecimal("u_waveStrength"), 0.002), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_waveStrength", x);
						return true;
					}),
				}),
			}),
			MenuItem("Quit Game", [engine] (Decimal) {
				engine->Quit();
				return false;
			}),
		};
		st.AddSystem<MenuSystem>(menu);

		IDType laserID;
		st.dtors.emplace_back(engine->AddObject(&laserID));
		engine->AddComponent<AudioSource>(laserID, assetM->Get<AudioAsset>("laser"), true);

		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 1.0, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<Renderer>().lock();
			renderer->SetUniform("u_red", x);
		}, 5.0 * 4 - 1.0, renderer->uniformsFloat["u_red"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<Renderer>().lock();
			renderer->SetUniform("u_green", x);
		}, 5.0 * 2 - 1.0, renderer->uniformsFloat["u_green"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<Renderer>().lock();
			renderer->SetUniform("u_blue", x);
		}, 5.0 - 1.0, renderer->uniformsFloat["u_blue"]));
	});

	engine.AddState("playing", [secsPerBeat, &playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("back", Transition{Pop(), Pop(), Push("world")});

		st.AddSystem<HumanPlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		auto renderer = engine->GetSystem<Renderer>().lock();

		st.dtors.emplace_back(inputM->On(Key::Escape,         [engine] (Key) { engine->transition = "back"; }));
		st.dtors.emplace_back(inputM->On(Key::ControllerBack, [engine] (Key) { engine->transition = "back"; }));

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("begin"));

		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("nexus"), LoopIn{3565397});

		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<Renderer>().lock();
			renderer->SetUniform("u_red", x);
		}, secsPerBeat * 4 - 0.5, renderer->uniformsFloat["u_red"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<Renderer>().lock();
			renderer->SetUniform("u_green", x);
		}, secsPerBeat * 2 - 0.5, renderer->uniformsFloat["u_green"]));
		st.dtors.emplace_back(tweener->Tween(0.5f, 1.0f, 0.5, true, [] (Polar *engine, const float &x) {
			auto renderer = engine->GetSystem<Renderer>().lock();
			renderer->SetUniform("u_blue", x);
		}, secsPerBeat - 0.5, renderer->uniformsFloat["u_blue"]));
	});

	engine.Run("root");
}
