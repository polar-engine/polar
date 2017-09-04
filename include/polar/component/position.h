#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

class PositionComponent : public Component {
public:
	Integrable<Point3> position;
	PositionComponent() : PositionComponent(Point3(0, 0, 0)) {}
	PositionComponent(const Point3 position) : position(position) {
		Add<IntegrableProperty>();
		auto component = Get<IntegrableProperty>().lock();
		if(component) {
			component->AddIntegrable(&this->position);
		}
	}
};
