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

enum class ConfigOption {
	BaseDetail,
	Grain,
	ScanIntensity,
	PixelFactor,
	VoxelFactor,
	Bloom,
	Cel,
	Mute
};

std::istream & operator>>(std::istream &s, ConfigOption &x) {
	std::string word;
	s >> word;

#define CASE(X) (word == #X) { x = ConfigOption::X; }
	if CASE(BaseDetail)
	else if CASE(Grain)
	else if CASE(ScanIntensity)
	else if CASE(PixelFactor)
	else if CASE(VoxelFactor)
	else if CASE(Bloom)
	else if CASE(Cel)
	else if CASE(Mute)
	else { assert(false, "unhandled ConfigOption"); }
#undef CASE

	return s;
}

std::ostream & operator<<(std::ostream &s, const ConfigOption &x) {
#define CASE(X) case ConfigOption::X: s << #X; break;
	switch(x) {
	CASE(BaseDetail)
	CASE(Grain)
	CASE(ScanIntensity)
	CASE(PixelFactor)
	CASE(VoxelFactor)
	CASE(Bloom)
	CASE(Cel)
	CASE(Mute)
	default: assert(false, "unhandled ConfigOption");
	}
#undef CASE

	return s;
}

using ConfigM = ConfigManager<ConfigOption>;

Freefall::Freefall(Polar &engine) {
	const double secsPerBeat = 1.2631578947368421;

	IDType playerID;

	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("world"), Push("notplaying"), Push("title")});

		st.AddSystem<AssetManager>();
		st.AddSystemAs<Renderer, GL32Renderer, const boost::container::vector<std::string> &>({ "perlin"/*, "fxaa", "bloom"*/ });
		st.AddSystem<AudioManager>();
		//st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<Tweener<float>>();
		st.AddSystem<ConfigM>("options.cfg");
		st.AddSystem<LevelSwitcher>();

		auto configM = engine->GetSystem<ConfigM>().lock();

		auto SetPipeline = [] (Polar *engine) {
			auto configM = engine->GetSystem<ConfigM>().lock();
			boost::container::vector<std::string> names = { "perlin" };
			if(configM->Get<bool>(ConfigOption::Bloom)) { names.emplace_back("bloom"); }
			if(configM->Get<bool>(ConfigOption::Cel)) { names.emplace_back("fxaa"); }
			engine->GetSystem<Renderer>().lock()->SetPipeline(names);
		};

		configM->On(ConfigOption::BaseDetail, [] (Polar *engine, ConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_baseDetail", x);
		});
		configM->On(ConfigOption::Grain, [] (Polar *engine, ConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_grain", x);
		});
		configM->On(ConfigOption::ScanIntensity, [] (Polar *engine, ConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_scanIntensity", x);
		});
		configM->On(ConfigOption::PixelFactor, [] (Polar *engine, ConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_pixelFactor", x);
		});
		configM->On(ConfigOption::VoxelFactor, [] (Polar *engine, ConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_voxelFactor", x);
		});
		configM->On(ConfigOption::Bloom, [SetPipeline] (Polar *engine, ConfigOption, bool bloom) { SetPipeline(engine); });
		configM->On(ConfigOption::Cel,   [SetPipeline] (Polar *engine, ConfigOption, bool bloom) { SetPipeline(engine); });
		configM->On(ConfigOption::Mute, [] (Polar *engine, ConfigOption, bool mute) {
			engine->GetSystem<AudioManager>().lock()->muted = mute;
		});

		configM->Set<Decimal>(ConfigOption::BaseDetail, 8);

		configM->Load();

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

		auto configM = engine->GetSystem<ConfigM>().lock();
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
					MenuItem("Base Detail", MenuControl::Slider<Decimal>(6, 30, configM->Get<Decimal>(ConfigOption::BaseDetail)), [engine] (Decimal x) {
						engine->GetSystem<ConfigM>().lock()->Set<Decimal>(ConfigOption::BaseDetail, x);
						return true;
					}),
					MenuItem("Bloom", MenuControl::Checkbox(configM->Get<bool>(ConfigOption::Bloom)), [engine] (Decimal state) {
						engine->GetSystem<ConfigM>().lock()->Set<bool>(ConfigOption::Bloom, state);
						return true;
					}),
					MenuItem("Cel", MenuControl::Checkbox(configM->Get<bool>(ConfigOption::Cel)), [engine] (Decimal state) {
						engine->GetSystem<ConfigM>().lock()->Set<bool>(ConfigOption::Cel, state);
						return true;
					}),
					MenuItem("Grain", MenuControl::Slider<Decimal>(0, 0.2, configM->Get<Decimal>(ConfigOption::Grain), 0.01), [engine] (Decimal x) {
						engine->GetSystem<ConfigM>().lock()->Set<Decimal>(ConfigOption::Grain, x);
						return true;
					}),
					MenuItem("Scanlines", MenuControl::Slider<Decimal>(0, 0.2, configM->Get<Decimal>(ConfigOption::ScanIntensity), 0.01), [engine] (Decimal x) {
						engine->GetSystem<ConfigM>().lock()->Set<Decimal>(ConfigOption::ScanIntensity, x);
						return true;
					}),
					//MenuItem("Precision", MenuControl::Selection({"Float", "Double"}), [] (Decimal) { return true; }),
					MenuItem("Pixel Factor", MenuControl::Slider<Decimal>(0, 20, configM->Get<Decimal>(ConfigOption::PixelFactor)), [engine] (Decimal x) {
						engine->GetSystem<ConfigM>().lock()->Set<Decimal>(ConfigOption::PixelFactor, x);
						return true;
					}),
					MenuItem("Voxel Factor", MenuControl::Slider<Decimal>(0, 20, configM->Get<Decimal>(ConfigOption::VoxelFactor)), [engine] (Decimal x) {
						engine->GetSystem<ConfigM>().lock()->Set<Decimal>(ConfigOption::VoxelFactor, x);
						return true;
					}),
					MenuItem("Show FPS", MenuControl::Checkbox(renderer->showFPS), [engine] (Decimal state) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->showFPS = state;
						return true;
					}),
				}),
				MenuItem("Audio", {
					MenuItem("Mute", MenuControl::Checkbox(configM->Get<bool>(ConfigOption::Mute)), [engine] (Decimal state) {
						engine->GetSystem<ConfigM>().lock()->Set<bool>(ConfigOption::Mute, state);
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
