#pragma once

#include <polar/asset/model.h>
#include <polar/component/base.h>

namespace polar::component {
	COMPONENT_BEGIN(model)
		struct vertex {
			math::point3 position;
			math::point3 normal;
			math::point2 texcoord;

			vertex() = default;
			vertex(math::point3 position, math::point3 normal, math::point2 texcoord)
			  : position(position), normal(normal), texcoord(texcoord) {}

			friend inline core::store_serializer &operator<<(core::store_serializer &s, const vertex &v) {
				return s << v.position << v.normal << v.texcoord;
			}

			friend inline core::store_deserializer &operator>>(core::store_deserializer &s, vertex &v) {
				return s >> v.position >> v.normal >> v.texcoord;
			}
		};

		struct triangle {
			vertex p;
			vertex q;
			vertex r;

			triangle() = default;
			triangle(vertex p, vertex q, vertex r) : p(p), q(q), r(r) {}

			friend inline core::store_serializer &operator<<(core::store_serializer &s, const triangle &t) {
				return s << t.p << t.q << t.r;
			}

			friend inline core::store_deserializer &operator>>(core::store_deserializer &s, triangle &t) {
				return s >> t.p >> t.q >> t.r;
			}
		};

		std::vector<triangle> triangles;

		model() = default;
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

		static std::shared_ptr<model> deserialize(core::store_deserializer &s) {
			auto c = std::make_shared<model>();
			s >> c->triangles;
			return c;
		}
	COMPONENT_END(model, model)
} // namespace polar::component
