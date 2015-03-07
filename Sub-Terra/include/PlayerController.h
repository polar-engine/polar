#pragma once

#include "System.h"

class PlayerController : public System {
protected:
	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;
	IDType object;

	virtual void InitObject() {};
	virtual void Init() override;
	virtual void Update(DeltaTicks &) override;
public:
	static bool IsSupported() { return true; }
	PlayerController(Polar *engine) : System(engine) {}
};
