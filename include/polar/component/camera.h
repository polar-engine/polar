#pragma once

#include <polar/component/base.h>

namespace polar::component {
	COMPONENT_BEGIN(camera)
		core::ref scene;
		core::ref target;
		wrapped_node<math::mat4x4> projection;

		camera(core::ref scene,
		       core::ref target,
		       wrapped_node<math::mat4x4> projection = math::mat4x4(1))
		  : scene(scene), target(target), projection(projection) {}

		bool serialize(core::store_serializer &s) const override {
			s << scene << target << projection;
			return true;
		}

		static std::shared_ptr<camera> deserialize(core::store_deserializer &s) {
			core::ref scene;
			core::ref target;

			s >> scene >> target;

			auto c = std::make_shared<camera>(scene, target);
			s >> c->projection;

			return c;
		}
	COMPONENT_END(camera, camera)
} // namespace polar::component
