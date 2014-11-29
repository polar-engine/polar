#pragma once

#include "System.h"

class Renderer : public System {
protected:
	uint16_t width;
	uint16_t height;
	float fovy;
	float zNear;
public:
	static bool IsSupported() { return false; }
	Renderer(Polar *engine) : System(engine) {}
	virtual void SetClearColor(const glm::fvec4 &) = 0;
	virtual void Use(const std::string &) = 0;
};
