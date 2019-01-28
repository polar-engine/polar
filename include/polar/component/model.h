#pragma once

#include <polar/component/base.h>
#include <polar/core/debugmanager.h>
#include <vector>

namespace polar::component {
	class model : public base {
	  public:
		using point_t   = glm::vec3;
		using tri_t     = std::tuple<point_t, point_t, point_t>;
		using point_vec = std::vector<point_t>;
		using tri_vec   = std::vector<tri_t>;

		GeometryType type;
		point_vec points;

		model() : type(GeometryType::None) {}
		model(GeometryType type, point_vec points)
		    : type(type), points(points) {}
		model(tri_vec triangles) : type(GeometryType::Triangles) {
			auto size = triangles.size() * 3;
			points.resize(size);
			for(tri_vec::size_type i = 0; i < triangles.size(); ++i) {
				auto &triangle    = triangles.at(i);
				points[i * 3 + 0] = std::get<0>(triangle);
				points[i * 3 + 1] = std::get<1>(triangle);
				points[i * 3 + 2] = std::get<2>(triangle);
			}
		}

		static auto calculate_normal(const point_t &p1, const point_t &p2,
		                             const point_t &p3) {
			auto deltaPos1 = p2 - p1;
			auto deltaPos2 = p3 - p1;

			point_t normal;
			normal.x = deltaPos1.y * deltaPos2.z - deltaPos1.z * deltaPos2.y;
			normal.y = deltaPos1.z * deltaPos2.x - deltaPos1.x * deltaPos2.z;
			normal.z = deltaPos1.x * deltaPos2.y - deltaPos1.y * deltaPos2.x;
			return glm::normalize(normal);
		}

		static auto calculate_normal(const tri_t &triangle) {
			return calculate_normal(std::get<0>(triangle),
			                        std::get<1>(triangle),
			                        std::get<2>(triangle));
		}

		auto calculate_normals() const {
			point_vec normals;
			normals.resize(points.size());

			bool reverse = false;
			debugmanager()->trace("calculate_normals:");

			point_vec::size_type i = 0;
			while(i <= points.size() - 3) {
				auto normal = reverse
					? calculate_normal(points[i], points[i + 2], points[i + 1])
					: calculate_normal(points[i], points[i + 1], points[i + 2]);
				normals[i + 0] = normal;
				normals[i + 1] = normal;
				normals[i + 2] = normal;

				debugmanager()->trace("points [", i, "] = ",     points [i]);
				debugmanager()->trace("normals[", i, "] = ",     normals[i]);
				debugmanager()->trace("points [", i + 1, "] = ", points [i + 1]);
				debugmanager()->trace("normals[", i + 1, "] = ", normals[i + 1]);
				debugmanager()->trace("points [", i + 2, "] = ", points [i + 2]);
				debugmanager()->trace("normals[", i + 2, "] = ", normals[i + 2]);

				switch(type) {
				case GeometryType::Triangles:
					i += 3;
					break;
				case GeometryType::TriangleStrip:
					++i;
					reverse = !reverse;
					break;
				default:
					debugmanager()->fatal(
					    "cannot calculate normals for non-triangle geometry");
					break;
				}
			}
			return normals;
		}
	};
} // namespace polar::component
