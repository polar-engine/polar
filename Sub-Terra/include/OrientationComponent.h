#pragma once

#include "Component.h"

class OrientationComponent : public Component {
public:
	glm::quat orientation;
	OrientationComponent() {}
	OrientationComponent(const Point3 &euler) : orientation(euler) {}
	OrientationComponent(const Point3 &&euler) : orientation(euler) {}
};
