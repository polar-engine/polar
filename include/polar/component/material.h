#pragma once

#include <polar/component/base.h>

namespace polar::component {
	COMPONENT_BEGIN(material)
		core::ref stage;
		std::optional<core::ref> diffuse;

		material(core::ref stage, decltype(diffuse) diffuse = {}) : stage(stage), diffuse(diffuse) {}

		bool serialize(core::store_serializer &s) const override {
			s << stage << diffuse;
			return true;
		}

		static std::shared_ptr<material> deserialize(core::store_deserializer &s) {
			core::ref stage;
			std::optional<core::ref> diffuse;

			s >> stage >> diffuse;

			auto c = std::make_shared<material>(stage, diffuse);
			return c;
		}
	COMPONENT_END(material, material)
} // namespace polar::component
