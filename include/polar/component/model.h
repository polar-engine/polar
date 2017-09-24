#pragma once

#include <polar/component/base.h>
#include <vector>

namespace polar { namespace component {
	class model : public base {
	public:
		typedef glm::vec3 PointType;
		typedef std::tuple<PointType, PointType, PointType> TriangleType;
		typedef std::vector<PointType> PointsType;
		typedef std::vector<TriangleType> TrianglesType;

		GeometryType type;
		PointsType points;

		model() : type(GeometryType::None) {}
		model(GeometryType type, PointsType points) : type(type), points(points) {}
		model(TrianglesType triangles) : type(GeometryType::Triangles) {
			auto size = triangles.size() * 3;
			points.resize(size);
			for(TrianglesType::size_type i = 0; i < triangles.size(); ++i) {
				auto &triangle = triangles.at(i);
				points[i * 3 + 0] = std::get<0>(triangle);
				points[i * 3 + 1] = std::get<1>(triangle);
				points[i * 3 + 2] = std::get<2>(triangle);
			}
		}

		PointsType calculate_normals() const {
			PointsType normals;
			normals.resize(points.size());

			PointsType::size_type i = 0;
			while(i < points.size() - 3) {
				auto normal = calculate_normal(points[i], points[i + 1], points[i + 2]);
				normals[i + 0] = normal;
				normals[i + 1] = normal;
				normals[i + 2] = normal;

				switch(type) {
				case GeometryType::Triangles:
					i += 3;
					break;
				case GeometryType::TriangleStrip:
					++i;
					break;
				default:
					debugmanager()->fatal("cannot calculate normals for non-triangle geometry");
					break;
				}
			}
			return normals;
		}

		static PointType calculate_normal(const TriangleType &triangle) {
			return calculate_normal(std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle));
		}
		static PointType calculate_normal(const PointType &p1, const PointType &p2, const PointType &p3) {
			PointType deltaPos1 = p2 - p1;
			PointType deltaPos2 = p3 - p1;

			PointType normal;
			normal.x = deltaPos1.y * deltaPos2.z - deltaPos1.z * deltaPos2.y;
			normal.y = deltaPos1.z * deltaPos2.x - deltaPos1.x * deltaPos2.z;
			normal.z = deltaPos1.x * deltaPos2.y - deltaPos1.y * deltaPos2.x;
			return glm::normalize(normal);
		}
	};
} }
