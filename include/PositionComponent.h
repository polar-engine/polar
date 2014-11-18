#pragma once

#include "Component.h"

class PositionComponent : public Component {
public:
	Point position;
	PositionComponent() : position(0, 0, 0, 1) {}
	PositionComponent(Point &position) : position(position) {}
	virtual ~PositionComponent() override {}
	static void * operator new(std::size_t size) {
		return ::operator new(size + (16 - size % 16));
	}
};
