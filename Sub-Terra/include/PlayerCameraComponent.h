#pragma once

#include "Component.h"

class PlayerCameraComponent : public Component {
public:
	Integrable<Point3> distance;
	Integrable<Point3> position;
	glm::quat orientation;
	PlayerCameraComponent(const Point3 &distance = Point3(0, 0, 0), const Point3 &position = Point3(0, 0, 0), const Point3 &euler = Point3(0, 0, 0))
		: distance(distance), position(position), orientation(glm::vec3(euler)) {
		Add<IntegrableProperty>();
		auto component = Get<IntegrableProperty>();
		if(component != nullptr) {
			component->AddIntegrable(&this->distance);
			component->AddIntegrable(&this->position);
		}
	}
};
