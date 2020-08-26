#pragma once

#include <polar/system/base.h>

namespace polar::system {
	class ttl : public base {
	  private:
		std::set<core::ref> objects;

		void update(DeltaTicks &dt) override {
			for(auto it = objects.begin(); it != objects.end();) {
				auto ptr = engine->get<component::ttl>(*it);
				ptr->accumulate(dt);
				if(!ptr->alive()) {
					it = objects.erase(it);
				} else {
					++it;
				}
			}
		}

	  public:
		static bool supported() { return true; }
		ttl(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "ttl"; }

		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> c) override {
			if(ti == typeid(component::ttl)) {
				objects.emplace(wr.own());
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti) override {
			if(ti == typeid(component::ttl)) {
				objects.erase(wr.own());
			}
		}
	};
} // namespace polar::system
