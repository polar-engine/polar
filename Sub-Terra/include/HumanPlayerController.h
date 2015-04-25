#pragma once

#include "PlayerController.h"

class HumanPlayerController : public PlayerController {
protected:
	Point2 orientVel;
	Point2 orientRot;
	bool rearView = false;
	std::uint_fast64_t heldBlocks = 0;

	virtual void Init() override;
	virtual void Update(DeltaTicks &) override;
public:
	static bool IsSupported() { return true; }
	HumanPlayerController(Polar *engine) : PlayerController(engine) {}
};