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
#include "CreditsSystem.h"
#include "TitlePlayerController.h"
#include "HumanPlayerController.h"
#include "Text.h"
#include "Level.h"

void Freefall::Run(const std::vector<std::string> &args) {
	const double secsPerBeat = 1.2631578947368421;

	Polar engine;

	IDType playerID, qID, eID;
	std::vector<std::string> levels;
	size_t levelIndex = 0;
	bool bloom = false;
	bool fxaa = false;

	srand((unsigned int)time(0));
	std::mt19937_64 rng(time(0));

	engine.AddState("root", [&levels, &levelIndex] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("world"), Push("notplaying"), Push("title")});

		//st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<Tweener<float>>();
		st.AddSystem<AudioManager>();
		st.AddSystemAs<Renderer, GL32Renderer, const boost::container::vector<std::string> &>({ "perlin"/*, "fxaa", "bloom"*/ });

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

		levels = assetM->List<Level>();
		levelIndex = 0;

		engine->transition = "forward";
	});

	engine.AddState("world", [&playerID, &levels, &levelIndex, &qID, &eID] (Polar *engine, EngineState &st) {
		auto assetM = engine->GetSystem<AssetManager>().lock();

		st.dtors.emplace_back(engine->AddObject(&playerID));

		Point3 seed = glm::ballRand(Decimal(1000.0));
		engine->AddComponent<PositionComponent>(playerID, seed);
		engine->AddComponent<OrientationComponent>(playerID);

		st.AddSystem<World>(assetM->Get<Level>(levels[levelIndex]), false);

		auto font = assetM->Get<FontAsset>("nasalization-rg");

		st.dtors.emplace_back(engine->AddObject(&qID));
		st.dtors.emplace_back(engine->AddObject(&eID));
		engine->AddComponentAs<Sprite, Text>(qID, font, "Q", Point2(20), Origin::TopLeft,  Point4(0.8902, 0.9647, 0.9922, 0));
		engine->AddComponentAs<Sprite, Text>(eID, font, "E", Point2(20), Origin::TopRight, Point4(0.8902, 0.9647, 0.9922, 0));
		engine->GetComponent<Sprite>(qID)->scale *= 0.5;
		engine->GetComponent<Sprite>(eID)->scale *= 0.5;

		auto qIndex = (levelIndex - 1) % levels.size();
		auto eIndex = (levelIndex + 1) % levels.size();
		engine->GetComponent<Sprite>(qID)->color.rgb() = assetM->Get<Level>(levels[qIndex])->keyframes.begin()->colors[0];
		engine->GetComponent<Sprite>(eID)->color.rgb() = assetM->Get<Level>(levels[eIndex])->keyframes.begin()->colors[0];
	});

	boost::shared_ptr<Destructor> soundDtor;;

	engine.AddState("notplaying", [&levels, &levelIndex, &qID, &eID, &soundDtor] (Polar *engine, EngineState &st) {
		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();

		IDType laserID;
		st.dtors.emplace_back(engine->AddObject(&laserID));
		engine->AddComponent<AudioSource>(laserID, assetM->Get<AudioAsset>("laser"), true);

		st.dtors.emplace_back(inputM->On(Key::Q, [engine, &levels, &levelIndex, &qID, &eID, &soundDtor] (Key) {
			auto assetM = engine->GetSystem<AssetManager>().lock();
			auto world = engine->GetSystem<World>().lock();
			levelIndex = (levelIndex - 1) % levels.size();
			world->SetLevel(assetM->Get<Level>(levels[levelIndex]));

			auto qIndex = (levelIndex - 1) % levels.size();
			auto eIndex = (levelIndex + 1) % levels.size();
			engine->GetComponent<Sprite>(qID)->color.rgb() = assetM->Get<Level>(levels[qIndex])->keyframes.begin()->colors[0];
			engine->GetComponent<Sprite>(eID)->color.rgb() = assetM->Get<Level>(levels[eIndex])->keyframes.begin()->colors[0];

			IDType soundID;
			soundDtor = engine->AddObject(&soundID);
			engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>("menu1"));
		}));

		st.dtors.emplace_back(inputM->On(Key::E, [engine, &levels, &levelIndex, &qID, &eID, &soundDtor] (Key) {
			auto assetM = engine->GetSystem<AssetManager>().lock();
			auto world = engine->GetSystem<World>().lock();
			levelIndex = (levelIndex + 1) % levels.size();
			world->SetLevel(assetM->Get<Level>(levels[levelIndex]));

			auto qIndex = (levelIndex - 1) % levels.size();
			auto eIndex = (levelIndex + 1) % levels.size();
			engine->GetComponent<Sprite>(qID)->color.rgb() = assetM->Get<Level>(levels[qIndex])->keyframes.begin()->colors[0];
			engine->GetComponent<Sprite>(eID)->color.rgb() = assetM->Get<Level>(levels[eIndex])->keyframes.begin()->colors[0];

			IDType soundID;
			soundDtor = engine->AddObject(&soundID);
			engine->AddComponent<AudioSource>(soundID, assetM->Get<AudioAsset>("menu1"));
		}));

		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		st.dtors.emplace_back(tweener->Tween(0, 1, 0.5, true, [&qID, &eID] (Polar *engine, const float &x) {
			auto alpha = glm::pow(Decimal(x), Decimal(0.75));
			engine->GetComponent<Sprite>(qID)->color.a = alpha;
			engine->GetComponent<Sprite>(eID)->color.a = alpha;
		}));
	});

	engine.AddState("title", [&playerID, &bloom, &fxaa] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{ Pop(), Pop(), Push("playing") });
		st.transitions.emplace("credits", Transition{ Pop(), Pop(), Push("credits") });
		st.transitions.emplace("back", Transition{ QuitAction() });

		st.AddSystem<TitlePlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
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
					MenuItem("Bloom", MenuControl::Checkbox(bloom), [engine, &bloom, &fxaa] (Decimal state) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						bloom = state;
						boost::container::vector<std::string> names = { "perlin" };
						if(bloom) { names.emplace_back("bloom"); }
						if(fxaa) { names.emplace_back("fxaa"); }
						renderer->MakePipeline(names);
						return true;
					}),
					MenuItem("FXAA", MenuControl::Checkbox(fxaa), [engine, &bloom, &fxaa] (Decimal state) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						fxaa = state;
						boost::container::vector<std::string> names = { "perlin" };
						if(bloom) { names.emplace_back("bloom"); }
						if(fxaa) { names.emplace_back("fxaa"); }
						renderer->MakePipeline(names);
						return true;
					}),
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
					MenuItem("Far Limiter", MenuControl::Slider<Decimal>(0, 3, renderer->GetUniformDecimal("u_farLimiter", 2), Decimal(0.1)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_farLimiter", x);
						return true;
					}),
					MenuItem("Far Focus", MenuControl::Slider<Decimal>(0.5, 1.5, renderer->GetUniformDecimal("u_farFocus", 1), Decimal(0.1)), [engine] (Decimal x) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->SetUniform("u_farFocus", x);
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
			}),
			MenuItem("Credits", [engine] (Decimal) {
				engine->transition = "credits";
				return false;
			}),
			MenuItem("Quit Game", [engine] (Decimal) {
				engine->Quit();
				return false;
			}),
		};
		st.AddSystem<MenuSystem>(menu);
	});

	engine.AddState("playing", [secsPerBeat, &playerID, &qID, &eID] (Polar *engine, EngineState &st) {
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

		st.dtors.emplace_back(tweener->Tween(1, 0, 0.2, false, [&qID, &eID] (Polar *engine, const float &x) {
			engine->GetComponent<Sprite>(qID)->color.a = x;
			engine->GetComponent<Sprite>(eID)->color.a = x;
		}));
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
		engine->AddComponentAs<Sprite, Text>(textID, font, "Game Over", Point2(0, 50), Origin::Center);

		std::ostringstream oss;
		oss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << seconds << 's';

		IDType timeID;
		st.dtors.emplace_back(engine->AddObject(&timeID));
		engine->AddComponentAs<Sprite, Text>(timeID, font, oss.str(), Point2(0, -100), Origin::Center);
		engine->GetComponent<Sprite>(timeID)->scale *= 0.75;

		IDType crashID;
		st.dtors.emplace_back(engine->AddObject(&crashID));
		engine->AddComponent<AudioSource>(crashID, assetM->Get<AudioAsset>("crash1"));

		IDType gameoverID;
		st.dtors.emplace_back(engine->AddObject(&gameoverID));
		engine->AddComponent<AudioSource>(gameoverID, assetM->Get<AudioAsset>("gameover"));
	});
	engine.AddState("credits", [&qID, &eID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("back", Transition{ Pop(), Push("notplaying"), Push("title") });

		Credits credits = {
			CreditsSection("Design and programming by", {
				"David Farrell",
			}),
			CreditsSection("Voice acting performed by",{
				"Christine Dodrill",
			}),
			CreditsSection("Sound effects created by", {
				"David Farrell",
			}),
			CreditsSection("Credits music composed by", {
				"David Farrell",
			}),
			CreditsSection("All other music written by", {
				"Alex \"aji\" Iadicicco",
			}),
			CreditsSection("Alpha testing volunteered by", {
				//"Aaron Dron?",
				//"Aidan Dodds",
				//"Assman",
				"Cengizhan Sayin",
				"Fangs124",
				//"Justin Kaufman",
				"Kitsune Curator",
				"Liquid Fear",
				"Mark M. Miller",
				//"Mark Miller + brother?",
				"Peter Black",
				"Shane Huberdeau",
				//"Simon Brand",
				"Sir Lad",
				"Sornaensis",
				//"Space Bread",
				"TechnoCrunch",
				"Tylor Froese",
				"Victor Fernandes",
				"Will Carroll",
				"Woffler",
				//"Xan",
			}),
			CreditsSection("Special thanks to", {
				"Bright",
				"darkf",
				"Miles Kjeller",
			}),
			CreditsSection("and", {
				"A big thanks to all my friends and family",
			}),
		};

		st.AddSystem<CreditsSystem>(credits);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("convergence"), true);

		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		st.dtors.emplace_back(tweener->Tween(1, 0, 0.2, false, [&qID, &eID] (Polar *engine, const float &x) {
			engine->GetComponent<Sprite>(qID)->color.a = x;
			engine->GetComponent<Sprite>(eID)->color.a = x;
		}));
	});

	engine.Run("root");
}
