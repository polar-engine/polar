#include <polar/core/state.h>

namespace polar::core {
	void state::init() {
		for(auto &system : orderedSystems) {
			auto &deref = *system;
			debugmanager()->debug("initing system: ", typeid(deref).name());
			system->init();
			debugmanager()->debug("inited system");
		}
	}

	void state::update(DeltaTicks &dt) {
		/* copy ordered systems vector to avoid invalidation */
		auto tmpSystems = orderedSystems;
		for(auto &system : tmpSystems) {
			auto &deref = *system;
			debugmanager()->trace("updating system: ", typeid(deref).name());
			system->update(dt);
			debugmanager()->trace("updated system");
		}
	}

	void state::component_added(IDType id, std::type_index ti,
	                            std::shared_ptr<component::base> ptr) {
		for(auto &pairSystem : *systems.get()) {
			auto &system = pairSystem.second;
			auto &deref  = *system;
			debugmanager()->trace("notifying system of component added: ",
			                      typeid(deref).name(), ", ", ti.name());
			system->componentadded(id, ti, ptr);
			debugmanager()->trace("notified system of component added");
		}
	}

	void state::component_removed(IDType id, std::type_index ti) {
		for(auto &pairSystem : *systems.get()) {
			auto &system = pairSystem.second;
			auto &deref  = *system;
			debugmanager()->trace("notifying system of component removed: ",
			                      typeid(deref).name(), ", ", ti.name());
			system->componentremoved(id, ti);
			debugmanager()->trace("notified system of component removed");
		}
	}
} // namespace polar::core
