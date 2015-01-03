#pragma once

#include "PlayerController.h"

class HumanPlayerController : public PlayerController {
protected:
	virtual void InitObject() override;
	virtual void Init() override;
	virtual void Update(DeltaTicks &, std::vector<Object *> &) override;
public:
	static bool IsSupported() { return true; }
	HumanPlayerController(Polar *engine) : PlayerController(engine) {}
};