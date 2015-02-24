#pragma once

#include "PlayerController.h"

class HumanPlayerController : public PlayerController {
protected:
	Point2 orientVel;

	virtual void InitObject() override;
	virtual void Init() override;
	virtual void Update(DeltaTicks &) override;
public:
	static bool IsSupported() { return true; }
	HumanPlayerController(Polar *engine) : PlayerController(engine) {}
};