#pragma once

#include "System.h"

class PlayerController : public System {
private:
	Object *object;
protected:
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
public:
	static bool IsSupported() { return true; }
	PlayerController(Polar *engine) : System(engine) {}
};
