#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	COMPONENT_BEGIN(color)
		template<typename T> using integrable = support::integrator::integrable<T>;

		integrable<math::point4> col;

		color(math::point4 col = math::point4(1)) : col(col) {
			add<property::integrable>();
			get<property::integrable>()->add(&this->col);
		}

		bool serialize(core::store_serializer &s) const override {
			s << col;
			return true;
		}

		static std::shared_ptr<color> deserialize(core::store_deserializer &s) {
			auto c = std::make_shared<color>();
			s >> c->col;
			return c;
		}
	COMPONENT_END(color, color)
} // namespace polar::component
