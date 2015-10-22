#pragma once

#include "PlayerController.h"

class HumanPlayerController : public PlayerController {
protected:
	Point2 orientVel;
	Point2 orientRot;
	long double accum = 0.0;
	float velocity = 10.0f;

	virtual void Init() override;
	virtual void Update(DeltaTicks &) override;
public:
	static bool IsSupported() { return true; }
	HumanPlayerController(Polar *engine) : PlayerController(engine) {}
};