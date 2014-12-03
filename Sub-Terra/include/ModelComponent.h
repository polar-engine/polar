#pragma once

#include "Component.h"

class ModelComponent : public Component {
public:
	GeometryType type;
	std::vector<Point> points;
	ModelComponent() : type(GeometryType::None) {}
	ModelComponent(GeometryType type, const std::vector<Point> &points) : type(type), points(points) {}
	ModelComponent(const std::vector<Triangle> &triangles) : type(GeometryType::Triangles) {
		points.resize(triangles.size() * 3);
		for(std::vector<Triangle>::size_type i = 0; i < triangles.size(); ++i) {
			auto &triangle = triangles.at(i);
			points.at(i * 3) = std::get<0>(triangle);
			points.at(i * 3 + 1) = std::get<1>(triangle);
			points.at(i * 3 + 2) = std::get<2>(triangle);
		}
	}
	virtual ~ModelComponent() override {}
};
