#pragma once

#include <polar/asset/model.h>
#include <polar/component/base.h>

namespace polar::component {
	class model : public base {
	  public:
		struct vertex {
			math::point3 position;
			math::point3 normal;
			math::point2 texcoord;

			vertex(math::point3 position, math::point3 normal, math::point2 texcoord)
			  : position(position), normal(normal), texcoord(texcoord) {}

			friend inline core::store_serializer &operator<<(core::store_serializer &s, const vertex &v) {
				return s << v.position << v.normal << v.texcoord;
			}
		};

		struct triangle {
			vertex p;
			vertex q;
			vertex r;

			triangle(vertex p, vertex q, vertex r) : p(p), q(q), r(r) {}

			friend inline core::store_serializer &operator<<(core::store_serializer &s, const triangle &t) {
				return s << t.p << t.q << t.r;
			}
		};

		std::vector<triangle> triangles;

		model(std::shared_ptr<asset::model> as) {
			for(auto &t : as->triangles) {
				auto p = vertex(t.p.position, t.p.normal, t.p.texcoord);
				auto q = vertex(t.q.position, t.q.normal, t.q.texcoord);
				auto r = vertex(t.r.position, t.r.normal, t.r.texcoord);
				triangles.emplace_back(p, q, r);
			}
		}

		bool serialize(core::store_serializer &s) const override {
			s << triangles;
			return true;
		}

		virtual std::string name() const override { return "model"; }
	};
} // namespace polar::component
