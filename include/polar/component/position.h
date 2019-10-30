#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>
#include <polar/core/debugmanager.h>

namespace polar::component {
	class position : public base {
		template<typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<Point3> pos;
		position(const Point3 pos = Point3(0)) : pos(pos) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->pos);
		}

		virtual std::string name() const override { return "position"; }

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
	};
} // namespace polar::component
