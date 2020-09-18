#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class renderable : public base {
	  public:
		core::ref scene;
		core::ref model;
		core::ref material;

		renderable(core::ref scene, core::ref model, core::ref material) : scene(scene), model(model), material(material) {}

		bool serialize(core::store_serializer &s) const override {
			s << scene << model << material;
			return true;
		}

		virtual std::string name() const override { return "renderable"; }
	};
} // namespace polar::component
