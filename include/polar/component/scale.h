#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

namespace polar::component {
	class scale : public base {
		template<typename T>
		using integrable = support::integrator::integrable<T>;

	  public:
		integrable<math::point3> sc;

		scale(math::point3 sc = math::point3(1)) : sc(sc) {
			add<property::integrable>();
			get<property::integrable>().lock()->add(&this->sc);
		}

		virtual std::string name() const override { return "scale"; }

		virtual accessor_list accessors() const override {
			accessor_list l;
			l.emplace_back("x", make_accessor<scale>(
				[] (scale *ptr) {
					return ptr->sc->x;
				},
				[] (scale *ptr, auto x) {
					ptr->sc->x = x;
				}
			));
			l.emplace_back("y", make_accessor<scale>(
				[] (scale *ptr) {
					return ptr->sc->y;
				},
				[] (scale *ptr, auto y) {
					ptr->sc->y = y;
				}
			));
			l.emplace_back("z", make_accessor<scale>(
				[] (auto ptr) {
					return ptr->sc->z;
				},
				[] (scale *ptr, auto z) {
					ptr->sc->z = z;
				}
			));
			return l;
		}
	};
} // namespace polar::component
