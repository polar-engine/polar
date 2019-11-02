#include <polar/core/polar.h>
#include <thread>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

namespace polar::core {
	polar::polar(std::vector<std::string> args) {
		srand((unsigned int)time(nullptr));

		for(auto &arg : args) {
			arguments.emplace(arg);

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
			} else if(arg == "-info") {
				debugmanager()->priority = priority_t::info;
			} else if(arg == "-notice") {
				debugmanager()->priority = priority_t::notice;
			} else if(arg == "-warning") {
				debugmanager()->priority = priority_t::warning;
			} else if(arg == "-error") {
				debugmanager()->priority = priority_t::error;
			} else if(arg == "-critical") {
				debugmanager()->priority = priority_t::critical;
			} else if(arg == "-fatal") {
				debugmanager()->priority = priority_t::fatal;
			}
		}

		debugmanager()->verbose("built on ", buildinfo_date(), " at ",
		                        buildinfo_time());
	}

	void polar::run(const std::string &initialState) {
		using namespace std::chrono_literals;

		running = true;

		stack.emplace_back(initialState, this);
		states[initialState].first(this, stack.back());
		stack.back().init();

		std::chrono::time_point<std::chrono::high_resolution_clock>
		    now = std::chrono::high_resolution_clock::now(),
		    then;

		uint64_t frameID = 0;
		while(running) {
			now = std::chrono::high_resolution_clock::now();
			DeltaTicksBase dtb = std::chrono::duration_cast<DeltaTicksBase>(now - then);

			// skip frame if no time elapsed
			if(dtb.count() > 0) {
				then += std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(dtb);
				DeltaTicks dt(dtb);

				debugmanager()->trace("frame #", frameID++, " (", dt.Ticks(), ')');

				for(auto &state : stack) { state.update(dt); }

				// perform transition at end of iteration to avoid invalidation
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
			std::this_thread::sleep_for(1ms);
		}
	}

	ref polar::add(IDType &inputID) {
		inputID = nextID++;
		return ref([this, inputID] { remove(inputID); });
	}

	void polar::insert(std::type_index ti, std::shared_ptr<system::base> ptr) {
		for(auto &state : stack) { state.system_added(ti, ptr); }
	}

	void polar::insert(IDType id, std::shared_ptr<component::base> component,
	                   std::type_index ti) {
		debugmanager()->trace("inserting component: ", ti.name());
		objects.insert(bimap::value_type(id, ti, component));
		for(auto &state : stack) { state.component_added(id, ti, component); }
		debugmanager()->trace("inserted component");
	}

	std::weak_ptr<system::base> polar::get(std::type_index ti) {
		for(auto &state : stack) {
			auto ptr = state.get(ti);
			if(!ptr.expired()) { return ptr; }
		}
		return std::weak_ptr<system::base>();
	}

	component::base *polar::get(IDType id, std::type_index ti) {
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

	void polar::remove(IDType id, std::type_index ti) {
		for(auto &state : stack) {
			state.component_removed(id, ti);
		}
		objects.erase(bimap::value_type(id, ti));
	}
} // namespace polar::core
