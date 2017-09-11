#include <iomanip>
#include <glm/gtc/random.hpp>
#include <polar/core/polar.h>
#include <polar/system/config.h>
#include <polar/system/asset.h>
#include <polar/system/event.h>
#include <polar/system/input.h>
#include <polar/system/work.h>
#include <polar/system/integrator.h>
#include <polar/system/tweener.h>
#include <polar/system/audio.h>
#include <polar/system/renderer/gl32.h>
#include <polar/system/world.h>
#include <polar/system/menu.h>
#include <polar/system/credits.h>
#include <polar/system/levelswitcher.h>
#include <polar/system/player/title.h>
#include <polar/system/player/human.h>
#include <polar/asset/level.h>
#include <polar/component/text.h>
#include <polar/component/playercamera.h>
#include <polar/fs/steam.h>
#include <freefall/app.h>
#include <freefall/config.h>

namespace freefall {
	using CloudConfigM = polar::system::config<CloudConfigOption, polar::fs::steam>;
	using LocalConfigM = polar::system::config<LocalConfigOption, polar::fs::local>;

	app::app(polar::core::polar &engine) {
		using namespace polar;
		using key_t = support::input::key;

		const double secsPerBeat = 1.2631578947368421;

		IDType playerID;

		engine.addstate("root", [] (core::polar *engine, core::state &st) {
			st.transitions.emplace("forward", Transition{Push("world"), Push("notplaying"), Push("title")});

			st.addsystem<system::asset>();
			st.addsystem_as<system::renderer::base, system::renderer::gl32, const std::vector<std::string> &>({ "perlin"/*, "fxaa", "bloom", "chroma"*/ });
			st.addsystem<system::audio>();
			//st.addsystem<system::work>();
			st.addsystem<system::event>();
			st.addsystem<system::input>();
			st.addsystem<system::integrator>();
			st.addsystem<system::tweener<float>>();
			st.addsystem<CloudConfigM>("options.cfg");
			debugmanager()->verbose("saved games directory is ", fs::local::savedgamesdir());
			st.addsystem<LocalConfigM>(fs::local::savedgamesdir() + "/local.cfg");
			st.addsystem<system::levelswitcher>();

			auto cloudConfigM = engine->getsystem<CloudConfigM>().lock();
			auto localConfigM = engine->getsystem<LocalConfigM>().lock();

			auto SetPipeline = [] (core::polar *engine) {
				auto cloudConfigM = engine->getsystem<CloudConfigM>().lock();
				auto localConfigM = engine->getsystem<LocalConfigM>().lock();

				std::vector<std::string> names = { "perlin" };
				if(cloudConfigM->get<Decimal>(CloudConfigOption::ChromaticAberration) > 0) { names.emplace_back("chroma"); }
				if(localConfigM->get<bool>(LocalConfigOption::Bloom)) { names.emplace_back("bloom"); }
				if(localConfigM->get<bool>(LocalConfigOption::Cel)) { names.emplace_back("fxaa"); }
				engine->getsystem<system::renderer::base>().lock()->setpipeline(names);
			};

			localConfigM->on(LocalConfigOption::BaseDetail, [] (core::polar *engine, LocalConfigOption, Decimal x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_baseDetail", x);
			});
			cloudConfigM->on(CloudConfigOption::Grain, [] (core::polar *engine, CloudConfigOption, Decimal x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_grain", x);
			});
			cloudConfigM->on(CloudConfigOption::ScanIntensity, [] (core::polar *engine, CloudConfigOption, Decimal x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_scanIntensity", x);
			});
			cloudConfigM->on(CloudConfigOption::ChromaticAberration, [SetPipeline] (core::polar *engine, CloudConfigOption, Decimal x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_aberration", x);
				SetPipeline(engine);
			});
			cloudConfigM->on(CloudConfigOption::PixelFactor, [] (core::polar *engine, CloudConfigOption, Decimal x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_pixelFactor", x);
			});
			cloudConfigM->on(CloudConfigOption::VoxelFactor, [] (core::polar *engine, CloudConfigOption, Decimal x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_voxelFactor", x);
			});
			localConfigM->on(LocalConfigOption::Fullscreen, [] (core::polar *engine, LocalConfigOption, bool fullscreen) {
				engine->getsystem<system::renderer::base>().lock()->setfullscreen(fullscreen);
			});
			localConfigM->on(LocalConfigOption::Bloom, [SetPipeline] (core::polar *engine, LocalConfigOption, bool bloom) { SetPipeline(engine); });
			localConfigM->on(LocalConfigOption::Cel,   [SetPipeline] (core::polar *engine, LocalConfigOption, bool bloom) { SetPipeline(engine); });

			cloudConfigM->on(CloudConfigOption::MouseSmoothing, [] (core::polar *engine, CloudConfigOption, Decimal x) {
				auto con = engine->getsystem<system::player::human>().lock();
				if(con) { con->smoothing = x; }
			});

			cloudConfigM->on(CloudConfigOption::Mute, [] (core::polar *engine, CloudConfigOption, bool mute) {
				engine->getsystem<system::audio>().lock()->muted = mute;
			});
			cloudConfigM->on(CloudConfigOption::MasterVolume, [] (core::polar *engine, CloudConfigOption, int x) {
				engine->getsystem<system::audio>().lock()->masterVolume = x;
			});
			cloudConfigM->on(CloudConfigOption::MusicVolume, [] (core::polar *engine, CloudConfigOption, int x) {
				engine->getsystem<system::audio>().lock()->volumes[static_cast<size_t>(support::audio::sourcetype::music)] = x;
			});
			cloudConfigM->on(CloudConfigOption::EffectVolume, [] (core::polar *engine, CloudConfigOption, int x) {
				engine->getsystem<system::audio>().lock()->volumes[static_cast<size_t>(support::audio::sourcetype::effect)] = x;
			});

			localConfigM->on(LocalConfigOption::UIScale, [] (core::polar *engine, LocalConfigOption, Decimal uiScale) {
				if(auto menuSystem = engine->getsystem<system::menu>().lock()) {
					menuSystem->uiScale = uiScale;
					menuSystem->render_all();
				}
			});

			localConfigM->set<Decimal>(LocalConfigOption::BaseDetail, 8);
			localConfigM->set<Decimal>(LocalConfigOption::UIScale, Decimal(0.3125));
			cloudConfigM->set<Decimal>(CloudConfigOption::MouseSmoothing, Decimal(0.995));
			cloudConfigM->set<int>(CloudConfigOption::MasterVolume, 100);
			cloudConfigM->set<int>(CloudConfigOption::MusicVolume, 100);
			cloudConfigM->set<int>(CloudConfigOption::EffectVolume, 100);

			cloudConfigM->load();
			localConfigM->load();

			/*assetM->get<asset::audio>("laser");
			assetM->get<asset::audio>("beep1");
			assetM->get<asset::audio>("menu1");
			assetM->get<asset::audio>("30");
			assetM->get<asset::audio>("60");
			assetM->get<asset::audio>("1");
			assetM->get<asset::audio>("2");
			assetM->get<asset::audio>("3");
			assetM->get<asset::audio>("4");
			assetM->get<asset::audio>("5");
			assetM->get<asset::audio>("6");
			assetM->get<asset::audio>("7");
			assetM->get<asset::audio>("8");
			assetM->get<asset::audio>("9");
			assetM->get<asset::audio>("seconds");
			assetM->get<asset::audio>("hundred");
			assetM->get<asset::audio>("fifty");
			assetM->get<asset::audio>("freefall");*/

			auto inputM = engine->getsystem<system::input>().lock();
			st.dtors.emplace_back(inputM->on(key_t::F, [engine] (key_t) {
				auto localConfigM = engine->getsystem<LocalConfigM>().lock();
				localConfigM->set<bool>(LocalConfigOption::Fullscreen, !localConfigM->get<bool>(LocalConfigOption::Fullscreen));
			}));
			st.dtors.emplace_back(inputM->on(key_t::M, [engine] (key_t) {
				auto cloudConfigM = engine->getsystem<CloudConfigM>().lock();
				cloudConfigM->set<bool>(CloudConfigOption::Mute, !cloudConfigM->get<bool>(CloudConfigOption::Mute));
			}));

			engine->transition = "forward";
		});

		engine.addstate("world", [&playerID] (core::polar *engine, core::state &st) {
			st.dtors.emplace_back(engine->addobject(&playerID));

			Point3 seed = glm::ballRand(WORLD_DECIMAL(1000.0));
			engine->addcomponent<component::position>(playerID, seed);
			engine->addcomponent<component::orientation>(playerID);

			st.addsystem<system::world>(engine->getsystem<system::levelswitcher>().lock()->getlevel(), false);

			engine->getsystem<system::renderer::base>().lock()->setuniform("u_exposure", Point3(1));
			auto tw = engine->getsystem<system::tweener<float>>().lock();
			st.dtors.emplace_back(tw->tween(1.0f, 0.0f, 0.5f, false, [] (core::polar *engine, float x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_exposure", Point3(-glm::pow(x, 2.0f)));
			}));
		});

		std::shared_ptr<core::destructor> soundDtor;

		engine.addstate("notplaying", [&soundDtor] (core::polar *engine, core::state &st) {
			engine->getsystem<system::input>().lock()->setactiveset("MenuControls");
			engine->getsystem<system::levelswitcher>().lock()->setenabled(true);
		});

		engine.addstate("title", [&playerID] (core::polar *engine, core::state &st) {
			st.transitions.emplace("forward", Transition{ Pop(), Pop(), Push("playing") });
			st.transitions.emplace("credits", Transition{ Pop(), Pop(), Push("credits") });
			st.transitions.emplace("back", Transition{ QuitAction() });

			st.addsystem<system::player::title>(playerID);

			auto cloudConfigM = engine->getsystem<CloudConfigM>().lock();
			auto localConfigM = engine->getsystem<LocalConfigM>().lock();
			auto renderer = engine->getsystem<system::renderer::base>().lock();

			auto assetM = engine->getsystem<system::asset>().lock();
			assetM->request<asset::audio>("nexus");
			assetM->request<asset::audio>("begin");
			assetM->request<asset::audio>("convergence");

			using menu_t = system::menuitem_vector_t;
			using menuitem = support::ui::menuitem;
			namespace control = support::ui::control;

			menu_t menu = {
				menuitem("Solo Play", [engine] (Decimal) {
					engine->transition = "forward";
					return false;
				}),
				menuitem("Options", {
					menuitem("Graphics", {
						menuitem("Base Detail", control::slider<Decimal>(6, 30, localConfigM->get<Decimal>(LocalConfigOption::BaseDetail)), [engine] (Decimal x) {
							engine->getsystem<LocalConfigM>().lock()->set<Decimal>(LocalConfigOption::BaseDetail, x);
							return true;
						}),
						menuitem("Bloom", control::checkbox(localConfigM->get<bool>(LocalConfigOption::Bloom)), [engine] (Decimal state) {
							engine->getsystem<LocalConfigM>().lock()->set<bool>(LocalConfigOption::Bloom, state);
							return true;
						}),
						menuitem("Cel", control::checkbox(localConfigM->get<bool>(LocalConfigOption::Cel)), [engine] (Decimal state) {
							engine->getsystem<LocalConfigM>().lock()->set<bool>(LocalConfigOption::Cel, state);
							return true;
						}),
						menuitem("Grain", control::slider<Decimal>(0, Decimal(0.2), cloudConfigM->get<Decimal>(CloudConfigOption::Grain), Decimal(0.01)), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<Decimal>(CloudConfigOption::Grain, x);
							return true;
						}),
						menuitem("Scanlines", control::slider<Decimal>(0, Decimal(0.2), cloudConfigM->get<Decimal>(CloudConfigOption::ScanIntensity), Decimal(0.01)), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<Decimal>(CloudConfigOption::ScanIntensity, x);
							return true;
						}),
						menuitem("Chromatic Aberration", control::slider<Decimal>(0, Decimal(0.001), cloudConfigM->get<Decimal>(CloudConfigOption::ChromaticAberration), Decimal(0.0001)), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<Decimal>(CloudConfigOption::ChromaticAberration, x);
							return true;
						}),
						//menuitem("Precision", control::Selection({"Float", "Double"}), [] (Decimal) { return true; }),
						menuitem("Pixel Factor", control::slider<Decimal>(0, 20, cloudConfigM->get<Decimal>(CloudConfigOption::PixelFactor)), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<Decimal>(CloudConfigOption::PixelFactor, x);
							return true;
						}),
						menuitem("Voxel Factor", control::slider<Decimal>(0, 20, cloudConfigM->get<Decimal>(CloudConfigOption::VoxelFactor)), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<Decimal>(CloudConfigOption::VoxelFactor, x);
							return true;
						}),
						menuitem("Fullscreen", control::checkbox(localConfigM->get<bool>(LocalConfigOption::Fullscreen)), [engine] (Decimal state) {
							engine->getsystem<LocalConfigM>().lock()->set<bool>(LocalConfigOption::Fullscreen, state);
							return true;
						}),
						// XXX: broken right now, fix me
						/*menuitem("UI Scale", control::slider<Decimal>(Decimal(0.125), Decimal(0.5), localConfigM->get<Decimal>(LocalConfigOption::UIScale), Decimal(0.03125)), [engine] (Decimal x) {
							engine->getsystem<LocalConfigM>().lock()->set<Decimal>(LocalConfigOption::UIScale, x);
							return true;
						}),*/
						menuitem("Show FPS", control::checkbox(renderer->showFPS), [engine] (Decimal state) {
							auto renderer = engine->getsystem<system::renderer::base>().lock();
							renderer->showFPS = state;
							return true;
						}),
					}),
					menuitem("Audio", {
						menuitem("Master Volume", control::slider<int>(0, 100, cloudConfigM->get<int>(CloudConfigOption::MasterVolume), 10), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<int>(CloudConfigOption::MasterVolume, x);
							return true;
						}),
						menuitem("Music Volume", control::slider<int>(0, 100, cloudConfigM->get<int>(CloudConfigOption::MusicVolume), 10), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<int>(CloudConfigOption::MusicVolume, x);
							return true;
						}),
						menuitem("Effect Volume", control::slider<int>(0, 100, cloudConfigM->get<int>(CloudConfigOption::EffectVolume), 10), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<int>(CloudConfigOption::EffectVolume, x);
							return true;
						}),
						menuitem("Mute", control::checkbox(cloudConfigM->get<bool>(CloudConfigOption::Mute)), [engine] (Decimal state) {
							engine->getsystem<CloudConfigM>().lock()->set<bool>(CloudConfigOption::Mute, state);
							return true;
						}),
					}),
					menuitem("Controls", {
						menuitem("Mouse Smoothing", control::slider<Decimal>(Decimal(0.9), Decimal(0.995), cloudConfigM->get<Decimal>(CloudConfigOption::MouseSmoothing), Decimal(0.005)), [engine] (Decimal x) {
							engine->getsystem<CloudConfigM>().lock()->set<Decimal>(CloudConfigOption::MouseSmoothing, x);
							return true;
						}),
					}),
				}),
				menuitem("Credits", [engine] (Decimal) {
					engine->transition = "credits";
					return false;
				}),
				menuitem("Quit Game", [engine] (Decimal) {
					engine->quit();
					return false;
				}),
			};
			auto uiScale = localConfigM->get<Decimal>(LocalConfigOption::UIScale);
			st.addsystem<system::menu>(uiScale, menu);
		});

		engine.addstate("playing", [secsPerBeat, &playerID] (core::polar *engine, core::state &st) {
			st.transitions.emplace("back", Transition{Pop(), Pop(), Push("world"), Push("notplaying"), Push("title")});
			st.transitions.emplace("gameover", Transition{Pop(), Push("notplaying"), Push("gameover")});

			st.addsystem<system::player::human>(playerID);

			auto cloudConfigM = engine->getsystem<CloudConfigM>().lock();
			engine->getsystem<system::player::human>().lock()->smoothing = cloudConfigM->get<Decimal>(CloudConfigOption::MouseSmoothing);

			auto assetM   = engine->getsystem<system::asset>().lock();
			auto inputM   = engine->getsystem<system::input>().lock();
			auto tw       = engine->getsystem<system::tweener<float>>().lock();
			auto renderer = engine->getsystem<system::renderer::base>().lock();

			inputM->setactiveset("InGameControls");

			engine->getsystem<system::world>().lock()->active = true;
			engine->getsystem<system::levelswitcher>().lock()->setenabled(false);

			for(auto k : { key_t::Escape, key_t::Backspace, key_t::MouseRight, key_t::ControllerBack }) {
				st.dtors.emplace_back(inputM->on(k, [engine] (key_t) { engine->transition = "gameover"; }));
			}

			st.dtors.emplace_back(inputM->ondigital("ingame_return", [engine] () { engine->transition = "gameover"; }));

			IDType beepID;
			st.dtors.emplace_back(engine->addobject(&beepID));
			engine->addcomponent<component::audiosource>(beepID, assetM->get<asset::audio>("begin"), support::audio::sourcetype::effect);

			IDType musicID;
			st.dtors.emplace_back(engine->addobject(&musicID));
			engine->addcomponent<component::audiosource>(musicID, assetM->get<asset::audio>("nexus"), support::audio::sourcetype::music, support::audio::loopin{3565397});

			engine->getsystem<system::renderer::base>().lock()->setmousecapture(true);
			st.dtors.emplace_back(std::make_shared<core::destructor>([engine] () {
				engine->getsystem<system::renderer::base>().lock()->setmousecapture(false);
			}));
		});

		engine.addstate("gameover", [] (core::polar *engine, core::state &st) {
			st.transitions.emplace("back", Transition{Pop(), Pop(), Pop(), Push("world"), Push("notplaying"), Push("title")});
			st.transitions.emplace("forward", Transition{Pop(), Pop(), Pop(), Push("world"), Push("playing")});

			auto assetM = engine->getsystem<system::asset>().lock();
			auto inputM = engine->getsystem<system::input>().lock();
			auto world  = engine->getsystem<system::world>().lock();

			for(auto k : { key_t::Space, key_t::Enter, key_t::MouseLeft, key_t::ControllerA }) {
				st.dtors.emplace_back(inputM->on(k, [engine] (key_t) { engine->transition = "forward"; }));
			}

			for(auto k : { key_t::Escape, key_t::Backspace, key_t::MouseRight, key_t::ControllerBack }) {
				st.dtors.emplace_back(inputM->on(k, [engine] (key_t) { engine->transition = "back"; }));
			}

			st.dtors.emplace_back(inputM->ondigital("menu_confirm", [engine] () { engine->transition = "forward"; }));
			st.dtors.emplace_back(inputM->ondigital("menu_back",    [engine] () { engine->transition = "back"; }));

			world->active = false;
			auto seconds = Decimal(world->get_ticks()) / ENGINE_TICKS_PER_SECOND;

			int32 totalSeconds;
			if(!SteamUserStats()->GetStat("time", &totalSeconds)) {
				debugmanager()->critical("failed to get current value of time stat");
			} else {
				debugmanager()->verbose("current value of time stat is ", totalSeconds);
			}

			totalSeconds += lround(seconds);

			debugmanager()->verbose("setting new value of time stat to ", totalSeconds);
			if(!SteamUserStats()->SetStat("time", totalSeconds)) {
				debugmanager()->critical("failed to set new value of time stat");
			}
			if(!SteamUserStats()->StoreStats()) {
				debugmanager()->critical("failed to upload new value of time stat");
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

			auto font = assetM->get<asset::font>("nasalization-rg");

			IDType textID;
			st.dtors.emplace_back(engine->addobject(&textID));
			engine->addcomponent<component::text>(textID, font, "Game Over");
			engine->addcomponent<component::screenposition>(textID, Point2(0, 50), support::ui::origin::center);

			std::ostringstream oss;
			oss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << seconds << 's';

			IDType timeID;
			st.dtors.emplace_back(engine->addobject(&timeID));
			engine->addcomponent<component::text>(timeID, font, oss.str());
			engine->addcomponent<component::screenposition>(timeID, Point2(0, -100), support::ui::origin::center);
			engine->addcomponent<component::scale>(timeID, Point3(0.75));

			IDType crashID;
			st.dtors.emplace_back(engine->addobject(&crashID));
			engine->addcomponent<component::audiosource>(crashID, assetM->get<asset::audio>("crash1"), support::audio::sourcetype::effect);

			IDType gameoverID;
			st.dtors.emplace_back(engine->addobject(&gameoverID));
			engine->addcomponent<component::audiosource>(gameoverID, assetM->get<asset::audio>("gameover"), support::audio::sourcetype::effect);

			auto tw = engine->getsystem<system::tweener<float>>().lock();
			st.dtors.emplace_back(tw->tween(0.0f, -1.0f, 0.5f, false, [] (core::polar *engine, float x) {
				engine->getsystem<system::renderer::base>().lock()->setuniform("u_exposure", Point3(x));
			}));
		});
		engine.addstate("credits", [] (core::polar *engine, core::state &st) {
			st.transitions.emplace("back", Transition{ Pop(), Push("notplaying"), Push("title") });

			using credits_t = system::credits_vector_t;
			using section_t = support::ui::credits_section;

			credits_t credits = {
				section_t("Design and programming by", {
					"David Farrell",
				}),
				section_t("Voice acting performed by",{
					"Christine Dodrill",
				}),
				section_t("Sound effects created by", {
					"David Farrell",
				}),
				section_t("Credits music composed by", {
					"David Farrell",
				}),
				section_t("All other music written by", {
					"Alex \"aji\" Iadicicco",
				}),
				section_t("Alpha testing volunteered by", {
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
				section_t("Special thanks to", {
					"Bright",
					"darkf",
					"Miles Kjeller",
				}),
				section_t("and", {
					"A big thanks to all my friends and family",
				}),
			};

			st.addsystem<system::credits>(credits);

			engine->getsystem<system::levelswitcher>().lock()->setenabled(false);

			auto assetM = engine->getsystem<system::asset>().lock();
			IDType musicID;
			st.dtors.emplace_back(engine->addobject(&musicID));
			engine->addcomponent<component::audiosource>(musicID, assetM->get<asset::audio>("convergence"), support::audio::sourcetype::music, true);
		});

		engine.run("root");
	}
}
