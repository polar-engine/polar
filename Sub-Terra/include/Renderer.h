#pragma once

#include <boost/container/vector.hpp>
#include "System.h"

class Renderer : public System {
protected:
	uint16_t width = 1280;
	uint16_t height = 720;
	Decimal fovPlus = glm::radians(10.0f);
	Decimal zNear = 0.05f;
	Decimal zFar = 48.0f;
	Decimal pixelDistanceFromScreen = 1000.0f;
public:
	boost::unordered_map<std::string, float> uniformsFloat;
	boost::unordered_map<std::string, Point3> uniformsPoint3;
	bool showFPS = false;

	static bool IsSupported() { return false; }
	Renderer(Polar *engine) : System(engine) {}

	virtual void MakePipeline(const boost::container::vector<std::string> &) = 0;
	virtual void SetClearColor(const Point4 &) = 0;
	virtual void SetUniform(const std::string &, float) = 0;
	virtual void SetUniform(const std::string &, Point3) = 0;
};
