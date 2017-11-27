#include <polar/core/polar.h>
#include <random>

namespace polar::core {
	polar::polar(std::vector<std::string> args) {
		srand((unsigned int)time(nullptr));
		std::mt19937_64 rng(time(nullptr));

		for(auto &arg : args) {
			if(arg == "-console") {
#if defined(_WIN32)
				AllocConsole();
				freopen("CONIN$", "r", stdin);
				freopen("CONOUT$", "w", stdout);
				freopen("CONOUT$", "w", stderr);
				std::wcout.clear();
				std::cout.clear();
				std::wcerr.clear();
				std::cerr.clear();
				std::wcin.clear();
				std::cin.clear();
#endif
			} else if(arg == "-trace") {
				debugmanager()->priority = priority_t::trace;
			} else if(arg == "-debug") {
				debugmanager()->priority = priority_t::debug;
			} else if(arg == "-verbose") {
				debugmanager()->priority = priority_t::verbose;
			}
		}

		debugmanager()->verbose("built on ", buildinfo_date(), " at ",
		                        buildinfo_time());
	}

	void polar::run(const std::string &initialState) {
		running = true;

		stack.emplace_back(initialState, this);
		states[initialState].first(this, stack.back());
		stack.back().init();

		std::chrono::time_point<std::chrono::high_resolution_clock>
		    now = std::chrono::high_resolution_clock::now(),
		    then;

		uint64_t frameID = 0;
		while(running) {
			then = now;
			now  = std::chrono::high_resolution_clock::now();
			DeltaTicks dt =
			    std::chrono::duration_cast<DeltaTicksBase>(now - then);

			debugmanager()->trace("frame #", frameID++, " (", dt.Ticks(), ')');

			for(auto &state : stack) { state.update(dt); }

			/* perform transition at end of iteration to avoid invalidation
			 */
			if(transition != "") {
				auto actions = stack.back().transitions[transition];
				transition   = "";
				for(auto &action : actions) {
					switch(action.type) {
					case StackActionType::Push:
						debugmanager()->debug("pushing state: ", action.name);
						stack.emplace_back(action.name, this);
						{
							debugmanager()->debug("calling state initializer");
							state &st = stack.back();
							debugmanager()->debug("calling state initializer");
							states[action.name].first(this, st);
						}
						debugmanager()->debug("pushed state");
						stack.back().init();
						break;
					case StackActionType::Pop: {
						auto &state = stack.back();
						debugmanager()->debug("popping state: ", state.name);
						states[state.name].second(this, state);
						stack.pop_back();
						debugmanager()->debug("popped state");
						break;
					}
					case StackActionType::Quit:
						quit();
						break;
					}
				}
			}
		}
	}

	std::shared_ptr<destructor> polar::add(IDType *inputID) {
		auto id  = nextID++;
		*inputID = id;
		return std::make_shared<destructor>([this, id]() { remove(id); });
	}

	void polar::insert(IDType id, std::shared_ptr<component::base> component,
	                   const std::type_info *ti) {
		debugmanager()->trace("inserting component: ", ti->name());
		objects.insert(bimap::value_type(id, ti, component));
		for(auto &state : stack) { state.component_added(id, ti, component); }
		debugmanager()->trace("inserted component");
	}

	std::weak_ptr<system::base> polar::get(const std::type_info *ti) {
		for(auto &state : stack) {
			auto ptr = state.get(ti);
			if(!ptr.expired()) { return ptr; }
		}
		return std::weak_ptr<system::base>();
	}

	component::base *polar::get(IDType id, const std::type_info *ti) {
		auto it = objects.find(bimap::relation(id, ti));
		if(it != objects.end()) {
			return it->info.get();
		} else {
			return nullptr;
		}
	}

	void polar::remove(IDType id) {
		auto pairLeft = objects.left.equal_range(id);
		for(auto it = pairLeft.first; it != pairLeft.second; ++it) {
			for(auto &state : stack) {
				state.component_removed(id, it->get_right());
			}
		}
		objects.left.erase(id);
	}
} // namespace polar::core
