#include "common.h"
#include <iomanip>
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
#include "Text.h"
#include "Level.h"

void Freefall::Run(const std::vector<std::string> &args) {
	const double secsPerBeat = 1.2631578947368421;
	Polar engine;
	IDType playerID;

	srand((unsigned int)time(0));
	std::mt19937_64 rng(time(0));

	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("world"), Push("notplaying"), Push("title")});

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
		st.dtors.emplace_back(engine->AddObject(&playerID));

		Point3 seed = glm::ballRand(Decimal(1000.0));
		engine->AddComponent<PositionComponent>(playerID, seed);
		engine->AddComponent<OrientationComponent>(playerID);

		Keyframe kf0(0);
		kf0.baseThreshold = Decimal(   0.8);
		kf0.beatTicks     = Decimal(1000.0);
		kf0.beatPower     = Decimal(   4.0);
		kf0.beatStrength  = Decimal(   0.005);
		kf0.waveTicks     = Decimal(2345.0);
		kf0.wavePower     = Decimal(   8.0);
		kf0.waveStrength  = Decimal(   0.02);
		kf0.worldScale    = Point3(20.0);
		kf0.colors        = {
			Point3(0.4, 1.0, 0.4),
			Point3(0.3, 0.9, 0.5),
			Point3(0.4, 0.8, 0.7)
		};
		kf0.colorTicks    = 10000.0;
		Keyframe kf10(10 * ENGINE_TICKS_PER_SECOND, kf0);
		kf10.baseThreshold = Decimal(0.75);
		Keyframe kf55(55 * ENGINE_TICKS_PER_SECOND, kf10);
		Keyframe kf55_1(55 * ENGINE_TICKS_PER_SECOND + 1, kf55);
		kf55_1.beatTicks = Decimal(20000.0);
		Keyframe kf60(60 * ENGINE_TICKS_PER_SECOND, kf55_1);
		kf60.baseThreshold = Decimal(0.6);
		kf60.beatPower     = Decimal(1.0);
		kf60.beatStrength  = Decimal(0.15);
		kf60.colors        = {
			Point3(1.0,  0.7, 0.1),
			Point3(0.95, 0.8, 0.3),
			Point3(0.9,  0.8, 0.6)
		};
		Keyframe kf90(90 * ENGINE_TICKS_PER_SECOND, kf60);
		Keyframe kf100(100 * ENGINE_TICKS_PER_SECOND, kf90);
		kf100.baseThreshold = Decimal(0.55);
		kf100.beatPower     = Decimal(4.0);
		kf100.beatStrength  = Decimal(0.002);
		kf100.colors        = {
			Point3(1.0, 0.3, 0.3),
			Point3(1.0, 0.7, 0.0),
			Point3(0.9, 0.3, 0.5)
		};
		Keyframe kf118(118 * ENGINE_TICKS_PER_SECOND, kf100);
		Keyframe kf120(120 * ENGINE_TICKS_PER_SECOND, kf118);
		kf120.baseThreshold = Decimal(0.75);
		kf120.beatPower     = Decimal(1.0);
		kf120.beatStrength  = Decimal(0.0);
		kf120.colors        = {
			Point3(0.4, 0.4, 1.0),
			Point3(0.5, 0.3, 0.9),
			Point3(0.7, 0.4, 0.8)
		};
		Keyframe kf138(138 * ENGINE_TICKS_PER_SECOND, kf120);
		Keyframe kf142(142 * ENGINE_TICKS_PER_SECOND, kf138);
		kf142.baseThreshold = Decimal(0.6);
		kf142.beatStrength  = Decimal(0.3);
		Keyframe kf155(155 * ENGINE_TICKS_PER_SECOND, kf142);
		Keyframe kf160(160 * ENGINE_TICKS_PER_SECOND, kf155);
		kf160.baseThreshold = Decimal(0.5);
		kf160.beatStrength  = Decimal(0.1);
		kf160.colors        = {
			Point3(1.0, 0.3, 0.3),
			Point3(1.0, 0.7, 0.0),
			Point3(0.9, 0.3, 0.5)
		};
		Level level({ kf0, kf10, kf55, kf55_1, kf60, kf90, kf100, kf118, kf120, kf138, kf142, kf155, kf160 });

		st.AddSystem<World>(level, false);
	});

	engine.AddState("notplaying", [] (Polar *engine, EngineState &st) {
		auto assetM = engine->GetSystem<AssetManager>().lock();
		IDType laserID;
		st.dtors.emplace_back(engine->AddObject(&laserID));
		engine->AddComponent<AudioSource>(laserID, assetM->Get<AudioAsset>("laser"), true);
	});

	engine.AddState("title", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{ Pop(), Pop(), Push("playing") });
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
					MenuItem("Base Detail", MenuControl::Slider<Decimal>(6, 40, renderer->GetUniformDecimal("u_baseDetail", 10)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_baseDetail", x);
						return true;
					}),
					//MenuItem("Far Detail", MenuControl::Slider<Decimal>(), [] (Decimal) { return true; }),
					//MenuItem("Far Limiter", MenuControl::Slider<Decimal>(), [] (Decimal) { return true; }),
					//MenuItem("Precision", MenuControl::Selection({"Float", "Double"}), [] (Decimal) { return true; }),
					MenuItem("Pixel Factor", MenuControl::Slider<Decimal>(0, 20, renderer->GetUniformDecimal("u_pixelFactor", 0)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_pixelFactor", x);
						return true;
					}),
					MenuItem("Voxel Factor", MenuControl::Slider<Decimal>(0, 20, renderer->GetUniformDecimal("u_voxelFactor", 0)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_voxelFactor", x);
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
				/*MenuItem("World", {
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
					MenuItem("u_worldScale.x", MenuControl::Slider<Decimal>(1, 100, renderer->GetUniformPoint3("u_worldScale").x, 0.5), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						auto p = renderer->GetUniformPoint3("u_worldScale");
						p.x = x;
						renderer->SetUniform("u_worldScale", p);
						return true;
					}),
					MenuItem("u_worldScale.y", MenuControl::Slider<Decimal>(1, 100, renderer->GetUniformPoint3("u_worldScale").y, 0.5), [engine] (Decimal y) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						auto p = renderer->GetUniformPoint3("u_worldScale");
						p.y = y;
						renderer->SetUniform("u_worldScale", p);
						return true;
					}),
					MenuItem("u_worldScale.z", MenuControl::Slider<Decimal>(1, 100, renderer->GetUniformPoint3("u_worldScale").z, 0.5), [engine] (Decimal z) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						auto p = renderer->GetUniformPoint3("u_worldScale");
						p.z = z;
						renderer->SetUniform("u_worldScale", p);
						return true;
					}),
				}),*/
			}),
			MenuItem("Quit Game", [engine] (Decimal) {
				engine->Quit();
				return false;
			}),
		};
		st.AddSystem<MenuSystem>(menu);
	});

	engine.AddState("playing", [secsPerBeat, &playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("back", Transition{Pop(), Pop(), Push("world"), Push("notplaying"), Push("title")});
		st.transitions.emplace("gameover", Transition{Pop(), Push("notplaying"), Push("gameover")});

		st.AddSystem<HumanPlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		auto renderer = engine->GetSystem<Renderer>().lock();
		engine->GetSystem<World>().lock()->active = true;

		for(auto k : { Key::Escape, Key::Backspace, Key::MouseRight, Key::ControllerBack }) {
			st.dtors.emplace_back(inputM->On(k, [engine] (Key) { engine->transition = "back"; }));
		}

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("begin"));

		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("nexus"), LoopIn{3565397});
	});

	engine.AddState("gameover", [] (Polar *engine, EngineState &st) {
		st.transitions.emplace("back", Transition{Pop(), Pop(), Pop(), Push("world"), Push("notplaying"), Push("title")});
		st.transitions.emplace("forward", Transition{Pop(), Pop(), Pop(), Push("world"), Push("playing")});

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto world = engine->GetSystem<World>().lock();

		for(auto k : { Key::Space, Key::Enter, Key::MouseLeft, Key::ControllerA }) {
			st.dtors.emplace_back(inputM->On(k, [engine] (Key) { engine->transition = "forward"; }));
		}

		for(auto k : { Key::Escape, Key::Backspace, Key::MouseRight, Key::ControllerBack }) {
			st.dtors.emplace_back(inputM->On(k, [engine] (Key) { engine->transition = "back"; }));
		}

		world->active = false;
		auto seconds = Decimal(world->GetTicks()) / ENGINE_TICKS_PER_SECOND;

		auto font = assetM->Get<FontAsset>("nasalization-rg");

		IDType textID;
		st.dtors.emplace_back(engine->AddObject(&textID));
		engine->AddComponent<Text>(textID, font, "Game Over", Point2(0), Origin::Center);

		std::ostringstream oss;
		oss << std::setiosflags(std::ios::fixed) << std::setprecision(1) << seconds << 's';

		IDType timeID;
		st.dtors.emplace_back(engine->AddObject(&timeID));
		engine->AddComponent<Text>(timeID, font, oss.str(), Point2(0, -150), Origin::Center);

		IDType beepID;
		st.dtors.emplace_back(engine->AddObject(&beepID));
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("gameover"));
	});

	engine.Run("root");
}
