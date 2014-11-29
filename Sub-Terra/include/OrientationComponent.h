#pragma once

#include "Component.h"

class OrientationComponent : public Component {
public:
	glm::quat orientation;
	OrientationComponent() {}
	OrientationComponent(Point &euler) : orientation(glm::vec3(euler)) {}
	OrientationComponent(Point &&euler) : orientation(glm::vec3(euler)) {}
};
