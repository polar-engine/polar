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
	virtual void MakePipeline(const boost::container::vector<std::string> &) = 0;
public:
	bool showFPS = false;

	static bool IsSupported() { return false; }
	Renderer(Polar *engine) : System(engine) {}

	inline uint16_t GetWidth() { return width; }
	inline uint16_t GetHeight() { return height; }

	virtual void SetPipeline(const boost::container::vector<std::string> &) = 0;
	virtual void SetClearColor(const Point4 &) = 0;
	virtual Decimal GetUniformDecimal(const std::string &, const Decimal = 0) = 0;
	virtual Point3 GetUniformPoint3(const std::string &, const Point3 = Point3(0)) = 0;
	virtual void SetUniform(const std::string &, Decimal, bool = false) = 0;
	virtual void SetUniform(const std::string &, Point3, bool = false) = 0;
};
