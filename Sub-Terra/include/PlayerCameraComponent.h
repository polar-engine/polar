#pragma once

#include "Component.h"

class PlayerCameraComponent : public Component {
public:
	Point distance;
	Point position;
	glm::quat orientation;
	PlayerCameraComponent(const Point &distance = Point(0, 0, 0, 1), const Point &position = Point(0, 0, 0, 1), const Point &euler = Point(0, 0, 0, 1))
		: distance(distance), position(position), orientation(glm::vec3(euler)) {}
};
