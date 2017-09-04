#pragma once

#include <polar/component/base.h>
#include <polar/property/integrable.h>

class ScaleComponent : public Component {
public:
	Integrable<Point3> scale;

	ScaleComponent(Point3 scale = Point3(1)) : scale(scale) {
		Add<IntegrableProperty>();
		Get<IntegrableProperty>().lock()->AddIntegrable(&this->scale);
	}
};
