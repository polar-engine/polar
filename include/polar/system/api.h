#pragma once

#include <polar/system/base.h>
#include <unordered_map>

namespace polar::system {
	class api : public base {
	  private:
		using system_accessor_bimap = boost::bimap<
			boost::bimaps::unordered_multiset_of<std::type_index>,
			boost::bimaps::unordered_multiset_of<std::string>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<system::base::accessor_type>>;
		using component_accessor_bimap = boost::bimap<
			boost::bimaps::unordered_multiset_of<std::type_index>,
			boost::bimaps::unordered_multiset_of<std::string>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<component::base::accessor_type>>;

		std::unordered_map<std::string, std::type_index> systems_by_name;
		system_accessor_bimap system_accessors;

		std::unordered_map<std::string, std::type_index> components_by_name;
		component_accessor_bimap component_accessors;
	  protected:
		void system_added(std::type_index ti, std::weak_ptr<system::base> ptr) override {
			auto sys = ptr.lock();

			systems_by_name.emplace(sys->name(), ti);

			for(auto &[name, accessor] : sys->accessors()) {
				system_accessors.insert(system_accessor_bimap::value_type(ti, name, accessor));
			}
		}

		void component_added(core::weak_ref, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			auto component = ptr.lock();

			components_by_name.emplace(component->name(), ti);

			for(auto &[name, accessor] : component->accessors()) {
				component_accessors.insert(component_accessor_bimap::value_type(ti, name, accessor));
			}
		}
	  public:
		static bool supported() { return true; }

		api(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "api"; }

		inline std::optional<std::type_index> get_system_by_name(std::string name) const {
			auto it = systems_by_name.find(name);
			if(it != systems_by_name.end()) {
				return it->second;
			} else {
				return {};
			}
		}

		inline std::optional<std::type_index> get_component_by_name(std::string name) const {
			auto it = components_by_name.find(name);
			if(it != components_by_name.end()) {
				return it->second;
			} else {
				return {};
			}
		}

		inline std::optional<system::base::accessor_type> get_system_accessor(std::type_index ti, std::string name) const {
			auto it = system_accessors.find(system_accessor_bimap::relation(ti, name));
			if(it != system_accessors.end()) {
				return it->info;
			} else {
				return {};
			}
		}

		inline std::optional<component::base::accessor_type> get_component_accessor(std::type_index ti, std::string name) const {
			auto it = component_accessors.find(component_accessor_bimap::relation(ti, name));
			if(it != component_accessors.end()) {
				return it->info;
			} else {
				return {};
			}
		}
	};
} // namespace polar::system
