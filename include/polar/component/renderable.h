#pragma once

#include <polar/component/base.h>

namespace polar::component {
	COMPONENT_BEGIN(renderable)
		core::ref scene;
		core::ref model;
		core::ref material;

		renderable(core::ref scene, core::ref model, core::ref material) : scene(scene), model(model), material(material) {}

		bool serialize(core::store_serializer &s) const override {
			s << scene << model << material;
			return true;
		}

		static std::shared_ptr<renderable> deserialize(core::store_deserializer &s) {
			core::ref scene;
			core::ref model;
			core::ref material;

			s >> scene >> model >> material;

			auto c = std::make_shared<renderable>(scene, model, material);
			return c;
		}
	COMPONENT_END(renderable, renderable)
} // namespace polar::component
