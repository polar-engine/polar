#pragma once

#include "Component.h"

class ScaleComponent : public Component {
public:
	Point3 scale;
	ScaleComponent() : scale(1) {}
	ScaleComponent(Point3 &scale) : scale(scale) {}
	virtual ~ScaleComponent() override {}
};
