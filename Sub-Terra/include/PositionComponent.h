#pragma once

#include "Component.h"

class PositionComponent : public Component {
public:
	Point position;
	PositionComponent() : position(0, 0, 0, 1) {}
	PositionComponent(const Point &position) : position(position) {}
	PositionComponent(Point &&position) : position(position) {}
	virtual ~PositionComponent() override {}
};
