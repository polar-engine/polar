#pragma once

#include <glm/gtc/random.hpp>
#include <polar/asset/material.h>
#include <polar/asset/model.h>
#include <polar/component/base.h>
#include <polar/core/log.h>
#include <vector>

namespace polar::component {
	class model : public base {
	  public:
		std::shared_ptr<asset::model> asset;

		model(decltype(asset) asset) : asset(asset) {}

		virtual std::string name() const override { return "model"; }

		void generate_normals() {
			for(auto &tri : asset->triangles) {
				auto delta1 = tri.q.position - tri.p.position;
				auto delta2 = tri.r.position - tri.p.position;

				math::point3 normal;
				normal.x = delta1.y * delta2.z - delta1.z * delta2.y;
				normal.y = delta1.z * delta2.x - delta1.x * delta2.z;
				normal.z = delta1.x * delta2.y - delta1.y * delta2.x;
				normal = glm::normalize(normal);

				if(tri.p.normal == math::point3(0, 0, 0)) {
					tri.p.normal = normal;
				}
				if(tri.q.normal == math::point3(0, 0, 0)) {
					tri.q.normal = normal;
				}
				if(tri.r.normal == math::point3(0, 0, 0)) {
					tri.r.normal = normal;
				}

				log()->trace("model", "position = ", tri.p.position);
				log()->trace("model", "normal   = ", tri.p.normal);
				log()->trace("model", "position = ", tri.q.position);
				log()->trace("model", "normal   = ", tri.q.normal);
				log()->trace("model", "position = ", tri.r.position);
				log()->trace("model", "normal   = ", tri.r.normal);
			}
		}
	};
} // namespace polar::component
