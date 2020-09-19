#pragma once

#include <polar/system/base.h>

namespace polar::system {
	class sched : public base {
	  private:
		std::map<core::weak_ref, std::set<core::weak_ref>> clocks;

		void update(DeltaTicks &dt) override {
			for(auto &pair : clocks) {
				auto clock = engine.mutate<component::clock::base>(pair.first);

				clock->accumulate(dt);

				while(clock->tick()) {
					for(auto &listener_ref : pair.second) {
						if(auto listener = engine.get<component::listener>(listener_ref)) {
							listener->trigger(clock->timestep);
						}
					}
				}
			}
		}

		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::listener)) {
				auto listener = std::static_pointer_cast<component::listener>(ptr.lock());

				if(auto clock = engine.get<component::clock::base>(listener->ref())) {
					clocks[listener->ref()].emplace(wr);
				}
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::listener)) {
				auto listener = std::static_pointer_cast<component::listener>(ptr.lock());

				auto search = clocks.find(listener->ref());
				if(search != clocks.end()) {
					search->second.erase(wr);
				}
			}
		}

	  public:
		static bool supported() { return true; }
		sched(core::polar &engine) : base(engine) {}

		virtual std::string name() const override { return "sched"; }
	};
} // namespace polar::system
