#include <polar/core/state.h>

namespace polar::core {
	void state::init() {
		for(auto &system : orderedSystems) {
			auto &deref = *system;
			log()->debug("core", "initing system: ", typeid(deref).name());
			system->init();
			log()->debug("core", "inited system");
		}
	}

	void state::update(DeltaTicks &dt) {
		for(auto &system : orderedSystems) {
			auto &deref = *system;
			log()->trace("core", "updating system: ", typeid(deref).name());
			system->update(dt);
			log()->trace("core", "updated system");
		}

		if(!toErase.empty()) {
			for(auto &system : toErase) {
				orderedSystems.erase(std::remove(orderedSystems.begin(), orderedSystems.end(), system));
			}
			toErase.clear();
		}
	}

	void state::insert(std::type_index ti, std::shared_ptr<system::base> ptr) {
		engine->insert(ti, ptr);
	}

	void state::system_added(std::type_index ti,
	                         std::shared_ptr<system::base> ptr) {
		for(auto &pairSystem : *systems.get()) {
			auto &system = pairSystem.second;
			auto &deref  = *system;
			log()->trace("core", "notifying system of system added: ", typeid(deref).name(), ", ", ti.name());
			system->system_added(ti, ptr);
			log()->trace("core", "notified system of system added");
		}
	}

	void state::component_added(weak_ref object, std::type_index ti, std::shared_ptr<component::base> ptr) {
		for(auto &pairSystem : *systems.get()) {
			auto &system = pairSystem.second;
			auto &deref  = *system;
			log()->trace("core", "notifying system of component added: ", typeid(deref).name(), ", ", ti.name());
			system->component_added(object, ti, ptr);
			log()->trace("core", "notified system of component added");
		}
	}

	void state::component_removed(weak_ref object, std::type_index ti) {
		for(auto &pairSystem : *systems.get()) {
			auto &system = pairSystem.second;
			auto &deref  = *system;
			log()->trace("core", "notifying system of component removed: ", typeid(deref).name(), ", ", ti.name());
			system->component_removed(object, ti);
			log()->trace("core", "notified system of component removed");
		}
	}
} // namespace polar::core
