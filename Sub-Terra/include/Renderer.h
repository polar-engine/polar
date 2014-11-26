#pragma once

#include "System.h"

class Renderer : public System {
public:
	static bool IsSupported() { return false; }
	Renderer(Polar *engine) : System(engine) {}
	virtual void SetClearColor(const glm::fvec4 &) = 0;
};
