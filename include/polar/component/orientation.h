#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	COMPONENT_BEGIN(orientation)
		template<typename... Ts> using integrable = support::integrator::integrable<Ts...>;

		integrable<math::quat, math::point3> orient;

		orientation(const math::quat orient = math::quat{1, 0, 0, 0}) : orient(orient) {
			add<property::integrable>();
			get<property::integrable>()->add(&this->orient);
		}

		bool serialize(core::store_serializer &s) const override {
			s << orient;
			return true;
		}

		static std::shared_ptr<orientation> deserialize(core::store_deserializer &s) {
			auto c = std::make_shared<orientation>();
			s >> c->orient;
			return c;
		}

		orientation(const math::point3 euler) : orientation(math::quat(euler)) {}
	COMPONENT_END(orientation, orientation)
} // namespace polar::component
