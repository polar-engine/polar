#include "common.h"
#include <iomanip>
#include <glm/gtc/random.hpp>
#include "Freefall.h"
#include "Polar.h"
#include "JobManager.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "ConfigManager.h"
#include "Integrator.h"
#include "Tweener.h"
#include "AudioManager.h"
#include "GL32Renderer.h"
#include "World.h"
#include "MenuSystem.h"
#include "CreditsSystem.h"
#include "LevelSwitcher.h"
#include "TitlePlayerController.h"
#include "HumanPlayerController.h"
#include "Text.h"
#include "Level.h"

enum class ConfigFloat : int {
	BaseDetail,
	Grain,
	PixelFactor,
	VoxelFactor,
	FarLimiter,
	FarFocus,
	ScanIntesity
};

enum class ConfigBool : int {
	Bloom,
	Cel,
	Mute
};

std::istream & operator>>(std::istream &s, ConfigFloat &x) {
	int y;
	s >> y;
	x = ConfigFloat(y);
	return s;
}

std::istream & operator>>(std::istream &s, ConfigBool &x) {
	int y;
	s >> y;
	x = ConfigBool(y);
	return s;
}

std::ostream & operator<<(std::ostream &s, const ConfigFloat x) {
	return s << int(x);
}

std::ostream & operator<<(std::ostream &s, const ConfigBool x) {
	return s << int(x);
}

using ConfigFloatM = ConfigManager<ConfigFloat, float>;
using ConfigBoolM = ConfigManager<ConfigBool, bool>;

void Freefall::Run(const std::vector<std::string> &args) {
	const double secsPerBeat = 1.2631578947368421;

	bool console = false;
	for(auto &arg : args) {
		if(arg == "-console") { console = true; }
		if(arg == "-trace")   { DebugManager()->priority = DebugPriority::Trace; }
		if(arg == "-debug")   { DebugManager()->priority = DebugPriority::Debug; }
		if(arg == "-verbose") { DebugManager()->priority = DebugPriority::Verbose; }
	}
	Polar engine;

	IDType playerID;

	srand((unsigned int)time(0));
	std::mt19937_64 rng(time(0));

	engine.AddState("root", [console] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("world"), Push("notplaying"), Push("title")});

		//st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<ConfigFloatM>("float.cfg", 0);
		st.AddSystem<ConfigBoolM>("bool.cfg", false);
		st.AddSystem<InputManager>();
		st.AddSystem<AssetManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<Tweener<float>>();
		st.AddSystem<AudioManager>();
		st.AddSystemAs<Renderer, GL32Renderer, const boost::container::vector<std::string> &>({ "perlin"/*, "fxaa", "bloom"*/ }, console);
		st.AddSystem<LevelSwitcher>();

		auto configFloatM = engine->GetSystem<ConfigFloatM>().lock();
		auto configBoolM = engine->GetSystem<ConfigBoolM>().lock();

		auto SetPipeline = [] (Polar *engine) {
			auto configBoolM = engine->GetSystem<ConfigBoolM>().lock();
			boost::container::vector<std::string> names = { "perlin" };
			if(configBoolM->Get(ConfigBool::Bloom)) { names.emplace_back("bloom"); }
			if(configBoolM->Get(ConfigBool::Cel)) { names.emplace_back("fxaa"); }
			engine->GetSystem<Renderer>().lock()->SetPipeline(names);
		};

		configFloatM->On(ConfigFloat::BaseDetail, [] (Polar *engine, ConfigFloat, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_baseDetail", x);
		});
		configFloatM->On(ConfigFloat::Grain, [] (Polar *engine, ConfigFloat, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_grain", x);
		});
		configFloatM->On(ConfigFloat::ScanIntesity, [] (Polar *engine, ConfigFloat, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_scanIntensity", x);
		});
		configFloatM->On(ConfigFloat::PixelFactor, [] (Polar *engine, ConfigFloat, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_pixelFactor", x);
		});
		configFloatM->On(ConfigFloat::VoxelFactor, [] (Polar *engine, ConfigFloat, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_voxelFactor", x);
		});
		configFloatM->On(ConfigFloat::FarLimiter, [] (Polar *engine, ConfigFloat, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_farLimiter", x);
		});
		configFloatM->On(ConfigFloat::FarFocus, [] (Polar *engine, ConfigFloat, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_farFocus", x);
		});
		configBoolM->On(ConfigBool::Bloom, [SetPipeline] (Polar *engine, ConfigBool, bool bloom) { SetPipeline(engine); });
		configBoolM->On(ConfigBool::Cel,   [SetPipeline] (Polar *engine, ConfigBool, bool bloom) { SetPipeline(engine); });
		configBoolM->On(ConfigBool::Mute, [] (Polar *engine, ConfigBool, bool mute) {
			engine->GetSystem<AudioManager>().lock()->muted = mute;
		});

		configFloatM->Set(ConfigFloat::BaseDetail, 10);
		configFloatM->Set(ConfigFloat::FarLimiter, 2);
		configFloatM->Set(ConfigFloat::FarFocus, 1);

		configFloatM->Load();
		configBoolM->Load();

		auto assetM = engine->GetSystem<AssetManager>().lock();
		assetM->Get<AudioAsset>("nexus");
		/*assetM->Get<AudioAsset>("laser");
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
		assetM->Get<AudioAsset>("freefall");*/

		engine->transition = "forward";
	});

	engine.AddState("world", [&playerID] (Polar *engine, EngineState &st) {
		auto assetM = engine->GetSystem<AssetManager>().lock();

		st.dtors.emplace_back(engine->AddObject(&playerID));

		Point3 seed = glm::ballRand(Decimal(1000.0));
		engine->AddComponent<PositionComponent>(playerID, seed);
		engine->AddComponent<OrientationComponent>(playerID);

		st.AddSystem<World>(engine->GetSystem<LevelSwitcher>().lock()->GetLevel(), false);

		engine->GetSystem<Renderer>().lock()->SetUniform("u_exposure", Point3(1));
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		st.dtors.emplace_back(tweener->Tween(1.0f, 0.0f, 0.5f, false, [] (Polar *engine, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_exposure", Point3(-glm::pow(x, 2.0f)));
		}));
	});

	boost::shared_ptr<Destructor> soundDtor;

	engine.AddState("notplaying", [&soundDtor] (Polar *engine, EngineState &st) {
		engine->GetSystem<InputManager>().lock()->SetActiveSet("MenuControls");
		engine->GetSystem<LevelSwitcher>().lock()->SetEnabled(true);
	});

	engine.AddState("title", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{ Pop(), Pop(), Push("playing") });
		st.transitions.emplace("credits", Transition{ Pop(), Pop(), Push("credits") });
		st.transitions.emplace("back", Transition{ QuitAction() });

		st.AddSystem<TitlePlayerController>(playerID);

		auto configFloatM = engine->GetSystem<ConfigFloatM>().lock();
		auto configBoolM = engine->GetSystem<ConfigBoolM>().lock();
		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto audioM = engine->GetSystem<AudioManager>().lock();
		auto renderer = engine->GetSystem<Renderer>().lock();

		Menu menu = {
			MenuItem("Solo Play", [engine] (Decimal) {
				engine->transition = "forward";
				return false;
			}),
			MenuItem("Options", {
				MenuItem("Graphics", {
					MenuItem("Base Detail", MenuControl::Slider<Decimal>(6, 30, configFloatM->Get(ConfigFloat::BaseDetail)), [engine] (Decimal x) {
						engine->GetSystem<ConfigFloatM>().lock()->Set(ConfigFloat ::BaseDetail, x);
						return true;
					}),
					MenuItem("Bloom", MenuControl::Checkbox(configBoolM->Get(ConfigBool::Bloom)), [engine] (Decimal state) {
						engine->GetSystem<ConfigBoolM>().lock()->Set(ConfigBool::Bloom, state);
						return true;
					}),
					MenuItem("Cel", MenuControl::Checkbox(configBoolM->Get(ConfigBool::Cel)), [engine] (Decimal state) {
						engine->GetSystem<ConfigBoolM>().lock()->Set(ConfigBool::Cel, state);
						return true;
					}),
					MenuItem("Grain", MenuControl::Slider<Decimal>(0, 0.2, configFloatM->Get(ConfigFloat::Grain), 0.01), [engine] (Decimal x) {
						engine->GetSystem<ConfigFloatM>().lock()->Set(ConfigFloat::Grain, x);
						return true;
					}),
					MenuItem("Scanlines", MenuControl::Slider<Decimal>(0, 0.2, configFloatM->Get(ConfigFloat::ScanIntesity), 0.01), [engine] (Decimal x) {
						engine->GetSystem<ConfigFloatM>().lock()->Set(ConfigFloat::ScanIntesity, x);
						return true;
					}),
					//MenuItem("Precision", MenuControl::Selection({"Float", "Double"}), [] (Decimal) { return true; }),
					MenuItem("Pixel Factor", MenuControl::Slider<Decimal>(0, 20, configFloatM->Get(ConfigFloat::PixelFactor)), [engine] (Decimal x) {
						engine->GetSystem<ConfigFloatM>().lock()->Set(ConfigFloat::PixelFactor, x);
						return true;
					}),
					MenuItem("Voxel Factor", MenuControl::Slider<Decimal>(0, 20, configFloatM->Get(ConfigFloat::VoxelFactor)), [engine] (Decimal x) {
						engine->GetSystem<ConfigFloatM>().lock()->Set(ConfigFloat::VoxelFactor, x);
						return true;
					}),
					MenuItem("Far Limiter", MenuControl::Slider<Decimal>(0, 3, configFloatM->Get(ConfigFloat::FarLimiter), Decimal(0.1)), [engine] (Decimal x) {
						engine->GetSystem<ConfigFloatM>().lock()->Set(ConfigFloat::FarLimiter, x);
						return true;
					}),
					MenuItem("Show FPS", MenuControl::Checkbox(renderer->showFPS), [engine] (Decimal state) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->showFPS = state;
						return true;
					}),
				}),
				MenuItem("Audio", {
					MenuItem("Mute", MenuControl::Checkbox(configBoolM->Get(ConfigBool::Mute)), [engine] (Decimal state) {
						engine->GetSystem<ConfigBoolM>().lock()->Set(ConfigBool::Mute, state);
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

	engine.AddState("playing", [secsPerBeat, &playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("back", Transition{Pop(), Pop(), Push("world"), Push("notplaying"), Push("title")});
		st.transitions.emplace("gameover", Transition{Pop(), Push("notplaying"), Push("gameover")});

		st.AddSystem<HumanPlayerController>(playerID);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto inputM = engine->GetSystem<InputManager>().lock();
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		auto renderer = engine->GetSystem<Renderer>().lock();

		inputM->SetActiveSet("InGameControls");

		engine->GetSystem<World>().lock()->active = true;
		engine->GetSystem<LevelSwitcher>().lock()->SetEnabled(false);

		for(auto k : { Key::Escape, Key::Backspace, Key::MouseRight, Key::ControllerBack }) {
			st.dtors.emplace_back(inputM->On(k, [engine] (Key) { engine->transition = "gameover"; }));
		}

		st.dtors.emplace_back(inputM->OnDigital("ingame_return", [engine] () { engine->transition = "gameover"; }));

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

		st.dtors.emplace_back(inputM->OnDigital("menu_confirm", [engine] () { engine->transition = "forward"; }));
		st.dtors.emplace_back(inputM->OnDigital("menu_back",    [engine] () { engine->transition = "back"; }));

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

		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		st.dtors.emplace_back(tweener->Tween(0.0f, -1.0f, 0.5f, false, [] (Polar *engine, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_exposure", Point3(x));
		}));
	});
	engine.AddState("credits", [] (Polar *engine, EngineState &st) {
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
				"Aidan Dodds",
				"AkariTakai",
				//"Assman",
				"Cengizhan Sayin",
				"DropTheBeat",
				"Fangs124",
				"Kitsune Curator",
				"Liquid Fear",
				"Mark M. Miller",
				//"Mark Miller + brother?",
				"Peter Black",
				"Shane Huberdeau",
				//"Simon Brand",
				"Sir Lad",
				"Sornaensis",
				"Space Bread",
				"TechnoCrunch",
				"theelous3",
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

		engine->GetSystem<LevelSwitcher>().lock()->SetEnabled(false);

		auto assetM = engine->GetSystem<AssetManager>().lock();
		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("convergence"), true);
	});

	engine.Run("root");
}
