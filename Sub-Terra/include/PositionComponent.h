#pragma once

#include "Component.h"
#include "IntegrableProperty.h"
#include "Integrable.h"

class PositionComponent : public Component {
public:
	Integrable<Point3> position;
	PositionComponent() : PositionComponent(Point3(0, 0, 0)) {}
	PositionComponent(const Point3 position) : position(position) {
		Add<IntegrableProperty>();
		auto component = Get<IntegrableProperty>();
		if(component != nullptr) {
			component->AddIntegrable(&this->position);
		}
	}
};
