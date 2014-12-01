#pragma once

#include "System.h"

class Renderer : public System {
protected:
	uint16_t width = 1280;
	uint16_t height = 720;
	float fovy = 70;
	float zNear = 0.05f;
public:
	static bool IsSupported() { return false; }
	Renderer(Polar *engine) : System(engine) {}
	virtual void SetClearColor(const glm::fvec4 &) = 0;
	virtual void Use(const std::string &) = 0;
};
