#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	COMPONENT_BEGIN(position)
		template<typename T>
		using integrable = support::integrator::integrable<T>;

		integrable<math::point3> pos;

		position(const math::point3 pos = math::point3(0)) : pos(pos) {
			add<property::integrable>();
			get<property::integrable>()->add(&this->pos);
		}

		bool serialize(core::store_serializer &s) const override {
			s << pos;
			return true;
		}

		static std::shared_ptr<position> deserialize(core::store_deserializer &s) {
			auto c = std::make_shared<position>();
			s >> c->pos;
			return c;
		}

		virtual accessor_list accessors() const override {
			accessor_list l;
			l.emplace_back("x", make_accessor<position>(
				[] (position *ptr) {
					return ptr->pos->x;
				},
				[] (position *ptr, auto x) {
					ptr->pos->x = x;
				}
			));
			l.emplace_back("y", make_accessor<position>(
				[] (position *ptr) {
					return ptr->pos->y;
				},
				[] (position *ptr, auto y) {
					ptr->pos->y = y;
				}
			));
			l.emplace_back("z", make_accessor<position>(
				[] (position *ptr) {
					return ptr->pos->z;
				},
				[] (position *ptr, auto z) {
					ptr->pos->z = z;
				}
			));
			return l;
		}
	COMPONENT_END(position, position)
} // namespace polar::component
