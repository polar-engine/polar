#pragma once

#include "Component.h"

class ScaleComponent : public Component {
public:
	Point scale;
	ScaleComponent() : scale(1) {}
	ScaleComponent(Point &scale) : scale(scale) {}
	virtual ~ScaleComponent() override {}
	static void * operator new(std::size_t size){
		return ::operator new(size + (16 - size % 16));
	}
};
