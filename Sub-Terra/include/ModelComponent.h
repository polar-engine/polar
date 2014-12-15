#pragma once

#include "Component.h"

typedef std::vector<Point> PointsType;

class ModelComponent : public Component {
public:
	GeometryType type;
	PointsType points;
	PointsType normals;

	ModelComponent() : type(GeometryType::None) {}
	ModelComponent(GeometryType type, const PointsType &points, const PointsType &normals)
		: type(type), points(points), normals(normals) {}
	ModelComponent(const std::vector<Triangle> &triangles) : type(GeometryType::Triangles) {
		auto size = triangles.size() * 3;
		points.resize(size);
		normals.resize(size);
		for(std::vector<Triangle>::size_type i = 0; i < triangles.size(); ++i) {
			auto &triangle = triangles.at(i);
			points.at(i * 3) = std::get<0>(triangle);
			points.at(i * 3 + 1) = std::get<1>(triangle);
			points.at(i * 3 + 2) = std::get<2>(triangle);

			Point deltaPos1 = std::get<1>(triangle) - std::get<0>(triangle);
			Point deltaPos2 = std::get<2>(triangle) - std::get<0>(triangle);

			Point normal;
			normal.x = deltaPos1.y * deltaPos2.z - deltaPos1.z * deltaPos2.y;
			normal.y = deltaPos1.z * deltaPos2.x - deltaPos1.x * deltaPos2.z;
			normal.z = deltaPos1.x * deltaPos2.y - deltaPos1.y * deltaPos2.x;
			normal = glm::normalize(normal);
			normals.at(i * 3) = normal;
			normals.at(i * 3 + 1) = normal;
			normals.at(i * 3 + 2) = normal;
		}
	}
};
