#pragma once

#include <polar/system/base.h>
#include <unordered_map>

namespace polar::system {
	class api : public base {
	  private:
		using accessor_bimap = boost::bimap<
			boost::bimaps::unordered_multiset_of<std::type_index>,
			boost::bimaps::unordered_multiset_of<std::string>,
			boost::bimaps::set_of_relation<>,
			boost::bimaps::with_info<component::base::accessor_type>>;

		std::unordered_map<std::string, std::type_index> components_by_name;
		accessor_bimap component_accessors;
	  protected:
		void componentadded(IDType, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			auto component = ptr.lock();

			components_by_name.emplace(component->name(), ti);

			for(auto &[name, accessor] : component->accessors()) {
				component_accessors.insert(accessor_bimap::value_type(ti, name, accessor));
			}
		}
	  public:
		static bool supported() { return true; }

		api(core::polar *engine) : base(engine) {}

		inline std::optional<std::type_index> get_component_by_name(std::string name) const {
			auto it = components_by_name.find(name);
			if(it != components_by_name.end()) {
				return it->second;
			} else {
				return {};
			}
		}

		inline std::optional<component::base::accessor_type> get_component_accessor(std::type_index ti, std::string name) const {
			auto it = component_accessors.find(accessor_bimap::relation(ti, name));
			if(it != component_accessors.end()) {
				return it->info;
			} else {
				return {};
			}
		}
	};
} // namespace polar::system
