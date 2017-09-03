#pragma once

#include "Component.h"
#include "IntegrableProperty.h"
#include "Integrable.h"

class PlayerCameraComponent : public Component {
public:
	Integrable<Point3> distance;
	Integrable<Point3> position;
	Quat orientation;
	PlayerCameraComponent(const Point3 &distance = Point3(0, 0, 0), const Point3 &position = Point3(0, 0, 0), const Point3 &euler = Point3(0, 0, 0))
		: distance(distance), position(position), orientation(Point3(euler)) {
		Add<IntegrableProperty>();
		auto component = Get<IntegrableProperty>().lock();
		if(component) {
			component->AddIntegrable(&this->distance);
			component->AddIntegrable(&this->position);
		}
	}
};
