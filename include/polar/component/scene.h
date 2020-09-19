#pragma once

#include <polar/component/base.h>

namespace polar::component {
	COMPONENT_BEGIN(scene)
		bool serialize(core::store_serializer &s) const override {
			return true;
		}

		static std::shared_ptr<scene> deserialize(core::store_deserializer &s) {
			auto c = std::make_shared<scene>();
			return c;
		}
	COMPONENT_END(scene, scene)
} // namespace polar::component
