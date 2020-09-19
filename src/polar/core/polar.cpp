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
				log()->priority = priority_t::trace;
			} else if(arg == "-debug") {
				log()->priority = priority_t::debug;
			} else if(arg == "-verbose") {
				log()->priority = priority_t::verbose;
			} else if(arg == "-info") {
				log()->priority = priority_t::info;
			} else if(arg == "-notice") {
				log()->priority = priority_t::notice;
			} else if(arg == "-warning") {
				log()->priority = priority_t::warning;
			} else if(arg == "-error") {
				log()->priority = priority_t::error;
			} else if(arg == "-critical") {
				log()->priority = priority_t::critical;
			} else if(arg == "-fatal") {
				log()->priority = priority_t::fatal;
			}
		}

		log()->verbose("core", "built on ", buildinfo_date(), " at ", buildinfo_time());
	}

	void polar::run(const std::string &initialState) {
		using namespace std::chrono_literals;

		running = true;

		stack.emplace_back(initialState, *this);
		states[initialState].first(*this, stack.back());
		stack.back().init();

		std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(),
		                                                            then;

		uint64_t frameID = 0;
		while(running) {
			now                = std::chrono::high_resolution_clock::now();
			DeltaTicksBase dtb = std::chrono::duration_cast<DeltaTicksBase>(now - then);

			// skip frame if no time elapsed
			if(dtb.count() > 0) {
				then += std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(dtb);
				DeltaTicks dt(dtb);

				log()->trace("core", "frame #", frameID++, " (", dt.Ticks(), ')');

				// send component notifications
				while(!components_to_notify.empty()) {
					auto &[r, ti] = components_to_notify.front();
					auto it = objects.get<index::rel_ordered>().find(relation{r, ti});
					if(it != objects.get<index::rel_ordered>().end()) {
						for(auto &state : stack) { state.component_added(r, ti, it->ptr); }
					}
					components_to_notify.pop();
				}

				for(auto &state : stack) { state.update(dt); }

				// perform deletions at end of iteration to avoid invalidation

				while(!components_to_remove.empty()) {
					auto &[r, ti] = components_to_remove.front();
					remove_now(r, ti);
					components_to_remove.pop();
				}

				while(!objects_to_remove.empty()) {
					auto &r = objects_to_remove.front();
					remove_now(r);
					objects_to_remove.pop();
				}

				// perform transition at end of iteration to avoid invalidation
				if(transition != "") {
					auto actions = stack.back().transitions[transition];
					transition   = "";
					for(auto &action : actions) {
						switch(action.type) {
						case StackActionType::Push:
							log()->debug("core", "pushing state: ", action.name);
							stack.emplace_back(action.name, *this);
							{
								log()->debug("core", "calling state initializer");
								state &st = stack.back();
								log()->debug("core", "calling state initializer");
								states[action.name].first(*this, st);
							}
							log()->debug("core", "pushed state");
							stack.back().init();
							break;
						case StackActionType::Pop: {
							auto &state = stack.back();
							log()->debug("core", "popping state: ", state.name);
							states[state.name].second(*this, state);
							stack.pop_back();
							log()->debug("core", "popped state");
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

	ref polar::add() {
		auto r    = ref(std::make_shared<destructor>());
		auto weak = weak_ref(r);
		r.dtor()->set([this, weak] { remove(weak); });
		log()->trace("new object ", r.id());
		return r;
	}

	void polar::insert(std::type_index ti, std::shared_ptr<system::base> ptr) {
		for(auto &state : stack) { state.system_added(ti, ptr); }
	}

	std::shared_ptr<component::base> polar::insert(weak_ref object, std::shared_ptr<component::base> component, std::type_index ti) {
		log()->trace("core", "inserting component: ", ti.name());

		auto [it, r] = objects.get<index::rel_ordered>().insert(relation{object, ti, component});
		components_to_notify.emplace(object, ti);

		log()->trace("core", "inserted component");

		return it->ptr;
	}

	std::weak_ptr<system::base> polar::get(std::type_index ti) {
		for(auto &state : stack) {
			auto ptr = state.get(ti);
			if(!ptr.expired()) { return ptr; }
		}
		return std::weak_ptr<system::base>();
	}

	std::shared_ptr<component::base> polar::get(weak_ref object, std::type_index ti) {
		auto it = objects.get<index::rel_ordered>().find(relation{object, ti});
		if(it != objects.get<index::rel_ordered>().end()) {
			return it->ptr;
		} else {
			return {};
		}
	}

	void polar::remove_now(weak_ref object) {
		auto pair = objects.get<index::ref>().equal_range(object);
		for(auto it = pair.first; it != pair.second; ++it) {
			for(auto &state : stack) { state.component_removed(object, it->ti, it->ptr); }
		}
		objects.get<index::ref>().erase(object);
	}

	void polar::remove_now(weak_ref object, std::type_index ti) {
		auto it = objects.get<index::rel_ordered>().find(relation{object, ti});

		if(it != objects.get<index::rel_ordered>().end()) {
			for(auto &state : stack) { state.component_removed(object, ti, it->ptr); }
			objects.get<index::rel_ordered>().erase(it);
		}
	}
} // namespace polar::core
