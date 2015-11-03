#pragma once

#include <boost/container/vector.hpp>
#include "Component.h"

class ModelComponent : public Component {
public:
	typedef boost::container::vector<Point3> PointsType;
	typedef boost::container::vector<Triangle> TrianglesType;

	GeometryType type;
	PointsType points;

	ModelComponent() : type(GeometryType::None) {}
	ModelComponent(GeometryType type, const PointsType &points) : type(type), points(points) {}
	ModelComponent(const TrianglesType &triangles) : type(GeometryType::Triangles) {
		auto size = triangles.size() * 3;
		points.resize(size);
		for(TrianglesType::size_type i = 0; i < triangles.size(); ++i) {
			auto &triangle = triangles.at(i);
			points[i * 3 + 0] = std::get<0>(triangle);
			points[i * 3 + 1] = std::get<1>(triangle);
			points[i * 3 + 2] = std::get<2>(triangle);
		}
	}

	PointsType CalculateNormals() const {
		PointsType normals;
		normals.resize(points.size());
		for(PointsType::size_type i = 0; i < points.size(); i += 3) {
			auto normal = CalculateNormal(points[i], points[i + 1], points[i + 2]);
			normals[i + 0] = normal;
			normals[i + 1] = normal;
			normals[i + 2] = normal;
		}
		return normals;
	}

	static Point3 CalculateNormal(const Triangle &triangle) {
		return CalculateNormal(std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle));
	}
	static Point3 CalculateNormal(const Point3 &p1, const Point3 &p2, const Point3 &p3) {
		Point3 deltaPos1 = p2 - p1;
		Point3 deltaPos2 = p3 - p1;

		Point3 normal;
		normal.x = deltaPos1.y * deltaPos2.z - deltaPos1.z * deltaPos2.y;
		normal.y = deltaPos1.z * deltaPos2.x - deltaPos1.x * deltaPos2.z;
		normal.z = deltaPos1.x * deltaPos2.y - deltaPos1.y * deltaPos2.x;
		return glm::normalize(normal);
	}
};
