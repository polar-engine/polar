#pragma once

#include <boost/container/vector.hpp>
#include "System.h"

class Renderer : public System {
protected:
	uint16_t width = 1280;
	uint16_t height = 720;
	float fovPlus = glm::radians(10.0f);
	float zNear = 0.05f;
	float zFar = 48.0f;
	float pixelDistanceFromScreen = 1000.0f;
public:
	static bool IsSupported() { return false; }
	Renderer(Polar *engine) : System(engine) {}
	virtual void SetClearColor(const Point4 &) = 0;
	virtual void MakePipeline(const boost::container::vector<std::string> &) = 0;
};
