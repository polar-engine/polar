#pragma once

#include "Component.h"

class ModelComponent : public Component {
public:
	std::vector<Point> points;
	ModelComponent() {}
	~ModelComponent() override {}
	ModelComponent(const std::vector<Point> &points) : points(points) {}
	ModelComponent(const std::vector<Triangle> &triangles) {
		points.resize(triangles.size() * 3);
		for(std::vector<Triangle>::size_type i = 0; i < triangles.size(); ++i) {
			auto &triangle = triangles.at(i);
			points.at(i) = std::get<0>(triangle);
			points.at(i + 1) = std::get<1>(triangle);
			points.at(i + 2) = std::get<2>(triangle);
		}
	}
};
