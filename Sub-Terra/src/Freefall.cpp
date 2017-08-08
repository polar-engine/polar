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
#include "SteamFS.h"
#include "FreefallConfig.h"

using SteamConfigM = ConfigManager<SteamConfigOption, SteamFS>;
using LocalConfigM = ConfigManager<LocalConfigOption, FileSystem>;

Freefall::Freefall(Polar &engine) {
	const double secsPerBeat = 1.2631578947368421;

	IDType playerID;

	engine.AddState("root", [] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{Push("world"), Push("notplaying"), Push("title")});

		st.AddSystem<AssetManager>();
		st.AddSystemAs<Renderer, GL32Renderer, const std::vector<std::string> &>({ "perlin"/*, "fxaa", "bloom"*/ });
		st.AddSystem<AudioManager>();
		//st.AddSystem<JobManager>();
		st.AddSystem<EventManager>();
		st.AddSystem<InputManager>();
		st.AddSystem<Integrator>();
		st.AddSystem<Tweener<float>>();
		st.AddSystem<SteamConfigM>("options.cfg");
		DebugManager()->Verbose("saved games directory is ", FileSystem::SavedGamesDir());
		st.AddSystem<LocalConfigM>(FileSystem::SavedGamesDir() + "/local.cfg");
		st.AddSystem<LevelSwitcher>();

		auto steamConfigM = engine->GetSystem<SteamConfigM>().lock();
		auto localConfigM = engine->GetSystem<LocalConfigM>().lock();

		auto SetPipeline = [] (Polar *engine) {
			auto localConfigM = engine->GetSystem<LocalConfigM>().lock();
			std::vector<std::string> names = { "perlin" };
			if(localConfigM->Get<bool>(LocalConfigOption::Bloom)) { names.emplace_back("bloom"); }
			if(localConfigM->Get<bool>(LocalConfigOption::Cel)) { names.emplace_back("fxaa"); }
			engine->GetSystem<Renderer>().lock()->SetPipeline(names);
		};

		localConfigM->On(LocalConfigOption::BaseDetail, [] (Polar *engine, LocalConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_baseDetail", x);
		});
		steamConfigM->On(SteamConfigOption::Grain, [] (Polar *engine, SteamConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_grain", x);
		});
		steamConfigM->On(SteamConfigOption::ScanIntensity, [] (Polar *engine, SteamConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_scanIntensity", x);
		});
		steamConfigM->On(SteamConfigOption::PixelFactor, [] (Polar *engine, SteamConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_pixelFactor", x);
		});
		steamConfigM->On(SteamConfigOption::VoxelFactor, [] (Polar *engine, SteamConfigOption, Decimal x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_voxelFactor", x);
		});
		localConfigM->On(LocalConfigOption::Fullscreen, [] (Polar *engine, LocalConfigOption, bool fullscreen) {
			engine->GetSystem<Renderer>().lock()->SetFullscreen(fullscreen);
		});
		localConfigM->On(LocalConfigOption::Bloom, [SetPipeline] (Polar *engine, LocalConfigOption, bool bloom) { SetPipeline(engine); });
		localConfigM->On(LocalConfigOption::Cel,   [SetPipeline] (Polar *engine, LocalConfigOption, bool bloom) { SetPipeline(engine); });

		steamConfigM->On(SteamConfigOption::MouseSmoothing, [] (Polar *engine, SteamConfigOption, Decimal x) {
			auto con = engine->GetSystem<HumanPlayerController>().lock();
			if(con) { con->smoothing = x; }
		});

		steamConfigM->On(SteamConfigOption::Mute, [] (Polar *engine, SteamConfigOption, bool mute) {
			engine->GetSystem<AudioManager>().lock()->muted = mute;
		});
		steamConfigM->On(SteamConfigOption::MasterVolume, [] (Polar *engine, SteamConfigOption, int x) {
			engine->GetSystem<AudioManager>().lock()->masterVolume = x;
		});
		steamConfigM->On(SteamConfigOption::MusicVolume, [] (Polar *engine, SteamConfigOption, int x) {
			engine->GetSystem<AudioManager>().lock()->volumes[static_cast<size_t>(AudioSourceType::Music)] = x;
		});
		steamConfigM->On(SteamConfigOption::EffectVolume, [] (Polar *engine, SteamConfigOption, int x) {
			engine->GetSystem<AudioManager>().lock()->volumes[static_cast<size_t>(AudioSourceType::Effect)] = x;
		});

		localConfigM->Set<Decimal>(LocalConfigOption::BaseDetail, 8);
		steamConfigM->Set<Decimal>(SteamConfigOption::MouseSmoothing, Decimal(0.995));
		steamConfigM->Set<int>(SteamConfigOption::MasterVolume, 100);
		steamConfigM->Set<int>(SteamConfigOption::MusicVolume, 100);
		steamConfigM->Set<int>(SteamConfigOption::EffectVolume, 100);

		steamConfigM->Load();
		localConfigM->Load();

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

		auto inputM = engine->GetSystem<InputManager>().lock();
		st.dtors.emplace_back(inputM->On(Key::F, [engine] (Key) {
			auto localConfigM = engine->GetSystem<LocalConfigM>().lock();
			localConfigM->Set<bool>(LocalConfigOption::Fullscreen, !localConfigM->Get<bool>(LocalConfigOption::Fullscreen));
		}));
		st.dtors.emplace_back(inputM->On(Key::M, [engine] (Key) {
			auto steamConfigM = engine->GetSystem<SteamConfigM>().lock();
			steamConfigM->Set<bool>(SteamConfigOption::Mute, !steamConfigM->Get<bool>(SteamConfigOption::Mute));
		}));

		engine->transition = "forward";
	});

	engine.AddState("world", [&playerID] (Polar *engine, EngineState &st) {
		auto assetM = engine->GetSystem<AssetManager>().lock();

		st.dtors.emplace_back(engine->AddObject(&playerID));

		Point3 seed = glm::ballRand(WORLD_DECIMAL(1000.0));
		engine->AddComponent<PositionComponent>(playerID, seed);
		engine->AddComponent<OrientationComponent>(playerID);

		st.AddSystem<World>(engine->GetSystem<LevelSwitcher>().lock()->GetLevel(), false);

		engine->GetSystem<Renderer>().lock()->SetUniform("u_exposure", Point3(1));
		auto tweener = engine->GetSystem<Tweener<float>>().lock();
		st.dtors.emplace_back(tweener->Tween(1.0f, 0.0f, 0.5f, false, [] (Polar *engine, float x) {
			engine->GetSystem<Renderer>().lock()->SetUniform("u_exposure", Point3(-glm::pow(x, 2.0f)));
		}));
	});

	std::shared_ptr<Destructor> soundDtor;

	engine.AddState("notplaying", [&soundDtor] (Polar *engine, EngineState &st) {
		engine->GetSystem<InputManager>().lock()->SetActiveSet("MenuControls");
		engine->GetSystem<LevelSwitcher>().lock()->SetEnabled(true);
	});

	engine.AddState("title", [&playerID] (Polar *engine, EngineState &st) {
		st.transitions.emplace("forward", Transition{ Pop(), Pop(), Push("playing") });
		st.transitions.emplace("credits", Transition{ Pop(), Pop(), Push("credits") });
		st.transitions.emplace("back", Transition{ QuitAction() });

		st.AddSystem<TitlePlayerController>(playerID);

		auto steamConfigM = engine->GetSystem<SteamConfigM>().lock();
		auto localConfigM = engine->GetSystem<LocalConfigM>().lock();
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
					MenuItem("Base Detail", MenuControl::Slider<Decimal>(6, 30, localConfigM->Get<Decimal>(LocalConfigOption::BaseDetail)), [engine] (Decimal x) {
						engine->GetSystem<LocalConfigM>().lock()->Set<Decimal>(LocalConfigOption::BaseDetail, x);
						return true;
					}),
					MenuItem("Bloom", MenuControl::Checkbox(localConfigM->Get<bool>(LocalConfigOption::Bloom)), [engine] (Decimal state) {
						engine->GetSystem<LocalConfigM>().lock()->Set<bool>(LocalConfigOption::Bloom, state);
						return true;
					}),
					MenuItem("Cel", MenuControl::Checkbox(localConfigM->Get<bool>(LocalConfigOption::Cel)), [engine] (Decimal state) {
						engine->GetSystem<LocalConfigM>().lock()->Set<bool>(LocalConfigOption::Cel, state);
						return true;
					}),
					MenuItem("Grain", MenuControl::Slider<Decimal>(0, Decimal(0.2), steamConfigM->Get<Decimal>(SteamConfigOption::Grain), Decimal(0.01)), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<Decimal>(SteamConfigOption::Grain, x);
						return true;
					}),
					MenuItem("Scanlines", MenuControl::Slider<Decimal>(0, Decimal(0.2), steamConfigM->Get<Decimal>(SteamConfigOption::ScanIntensity), Decimal(0.01)), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<Decimal>(SteamConfigOption::ScanIntensity, x);
						return true;
					}),
					//MenuItem("Precision", MenuControl::Selection({"Float", "Double"}), [] (Decimal) { return true; }),
					MenuItem("Pixel Factor", MenuControl::Slider<Decimal>(0, 20, steamConfigM->Get<Decimal>(SteamConfigOption::PixelFactor)), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<Decimal>(SteamConfigOption::PixelFactor, x);
						return true;
					}),
					MenuItem("Voxel Factor", MenuControl::Slider<Decimal>(0, 20, steamConfigM->Get<Decimal>(SteamConfigOption::VoxelFactor)), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<Decimal>(SteamConfigOption::VoxelFactor, x);
						return true;
					}),
					MenuItem("Fullscreen", MenuControl::Checkbox(localConfigM->Get<bool>(LocalConfigOption::Fullscreen)), [engine] (Decimal state) {
						engine->GetSystem<LocalConfigM>().lock()->Set<bool>(LocalConfigOption::Fullscreen, state);
						return true;
					}),
					MenuItem("Show FPS", MenuControl::Checkbox(renderer->showFPS), [engine] (Decimal state) {
						auto renderer = engine->GetSystem<Renderer>().lock();
						renderer->showFPS = state;
						return true;
					}),
				}),
				MenuItem("Audio", {
					MenuItem("Master Volume", MenuControl::Slider<int>(0, 100, steamConfigM->Get<int>(SteamConfigOption::MasterVolume), 10), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<int>(SteamConfigOption::MasterVolume, x);
						return true;
					}),
					MenuItem("Music Volume", MenuControl::Slider<int>(0, 100, steamConfigM->Get<int>(SteamConfigOption::MusicVolume), 10), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<int>(SteamConfigOption::MusicVolume, x);
						return true;
					}),
					MenuItem("Effect Volume", MenuControl::Slider<int>(0, 100, steamConfigM->Get<int>(SteamConfigOption::EffectVolume), 10), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<int>(SteamConfigOption::EffectVolume, x);
						return true;
					}),
					MenuItem("Mute", MenuControl::Checkbox(steamConfigM->Get<bool>(SteamConfigOption::Mute)), [engine] (Decimal state) {
						engine->GetSystem<SteamConfigM>().lock()->Set<bool>(SteamConfigOption::Mute, state);
						return true;
					}),
				}),
				MenuItem("Controls", {
					MenuItem("Mouse Smoothing", MenuControl::Slider<Decimal>(Decimal(0.9), Decimal(0.995), steamConfigM->Get<Decimal>(SteamConfigOption::MouseSmoothing), Decimal(0.005)), [engine] (Decimal x) {
						engine->GetSystem<SteamConfigM>().lock()->Set<Decimal>(SteamConfigOption::MouseSmoothing, x);
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

		auto steamConfigM = engine->GetSystem<SteamConfigM>().lock();
		engine->GetSystem<HumanPlayerController>().lock()->smoothing = steamConfigM->Get<Decimal>(SteamConfigOption::MouseSmoothing);

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
		engine->AddComponent<AudioSource>(beepID, assetM->Get<AudioAsset>("begin"), AudioSourceType::Effect);

		IDType musicID;
		st.dtors.emplace_back(engine->AddObject(&musicID));
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("nexus"), AudioSourceType::Music, LoopIn{3565397});

		engine->GetSystem<Renderer>().lock()->SetMouseCapture(true);
		st.dtors.emplace_back(std::make_shared<Destructor>([engine] () { engine->GetSystem<Renderer>().lock()->SetMouseCapture(false); }));
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

		int32 totalSeconds;
		if(!SteamUserStats()->GetStat("time", &totalSeconds)) {
			DebugManager()->Critical("failed to get current value of time stat");
		} else {
			DebugManager()->Verbose("current value of time stat is ", totalSeconds);
		}

		totalSeconds += lround(seconds);

		DebugManager()->Verbose("setting new value of time stat to ", totalSeconds);
		if(!SteamUserStats()->SetStat("time", totalSeconds)) {
			DebugManager()->Critical("failed to set new value of time stat");
		}
		if(!SteamUserStats()->StoreStats()) {
			DebugManager()->Critical("failed to upload new value of time stat");
		}

#define IndicateAchievement(SZ, MIN, MAX) \
		if(totalSeconds > MIN && totalSeconds < MAX) { \
			SteamUserStats()->IndicateAchievementProgress(SZ, totalSeconds, MAX); \
		}

		     IndicateAchievement("1000_time_minutes_1",       0,       60)
		else IndicateAchievement("1010_time_minutes_2",      60,      120)
		else IndicateAchievement("1020_time_minutes_5",     120,      300)
		else IndicateAchievement("1030_time_minutes_15",    300,      900)
		else IndicateAchievement("1040_time_minutes_30",    900,     1800)
		else IndicateAchievement("1050_time_hours_1",      1800,     3600)
		else IndicateAchievement("1060_time_hours_2",      3600,     7200)
		else IndicateAchievement("1070_time_hours_6",      7200,    21600)
		else IndicateAchievement("1080_time_hours_12",    21600,    43200)
		else IndicateAchievement("1090_time_days_1",      43200,    86400)
		else IndicateAchievement("1100_time_weeks_1",     86400,   604800)
		else IndicateAchievement("1110_time_years_1",    604800, 31536000)

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
		engine->AddComponent<AudioSource>(crashID, assetM->Get<AudioAsset>("crash1"), AudioSourceType::Effect);

		IDType gameoverID;
		st.dtors.emplace_back(engine->AddObject(&gameoverID));
		engine->AddComponent<AudioSource>(gameoverID, assetM->Get<AudioAsset>("gameover"), AudioSourceType::Effect);

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
		engine->AddComponent<AudioSource>(musicID, assetM->Get<AudioAsset>("convergence"), AudioSourceType::Music, true);
	});

	engine.Run("root");
}
