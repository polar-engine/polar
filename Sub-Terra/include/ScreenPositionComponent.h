#pragma once

#include "Component.h"
#include "IntegrableProperty.h"

enum class Origin {
	BottomLeft,
	BottomRight,
	TopLeft,
	TopRight,
	Left,
	Right,
	Bottom,
	Top,
	Center
};

class ScreenPositionComponent : public Component {
public:
	Integrable<Point2> position;
	Origin origin;

	ScreenPositionComponent(Point2 position = Point2(0), Origin origin = Origin::BottomLeft) : position(position), origin(origin) {
		Add<IntegrableProperty>();
		Get<IntegrableProperty>().lock()->AddIntegrable(&this->position);
	}
};
