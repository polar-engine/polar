#pragma once

#include "Component.h"

class PhysicalComponent : public Component {
public:
	float mass = 1.0f;
	float hardness = 1.0f;
	float sharpness = 0.0f;
	float durability = std::numeric_limits<float>::infinity();

	PhysicalComponent() {}
	PhysicalComponent(const float &mass, const float &hardness, const float &sharpness, const float &durability = std::numeric_limits<float>::infinity())
		: mass(mass), hardness(hardness), sharpness(sharpness), durability(durability) {}
};
