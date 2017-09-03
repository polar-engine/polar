#pragma once

#include "Component.h"
#include "IntegrableProperty.h"

class ColorComponent : public Component {
public:
	Integrable<Point4> color;

	ColorComponent(Point4 color = Point4(1)) : color(color) {
		Add<IntegrableProperty>();
		Get<IntegrableProperty>().lock()->AddIntegrable(&this->color);
	}
};
