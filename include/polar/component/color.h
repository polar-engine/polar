#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

class ColorComponent : public Component {
public:
	Integrable<Point4> color;

	ColorComponent(Point4 color = Point4(1)) : color(color) {
		Add<IntegrableProperty>();
		Get<IntegrableProperty>().lock()->AddIntegrable(&this->color);
	}
};
