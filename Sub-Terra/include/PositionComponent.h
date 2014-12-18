#pragma once

#include "Component.h"
#include "IntegrableProperty.h"
#include "Integrable.h"

class PositionComponent : public Component {
public:
	Integrable<Point> position;
	PositionComponent() : PositionComponent(Point(0, 0, 0, 1)) {}
	PositionComponent(const Point position) : position(position) {
		Add<IntegrableProperty>();
		auto component = Get<IntegrableProperty>();
		if(component != nullptr) {
			component->AddIntegrable(&this->position);
		}
	}
};
