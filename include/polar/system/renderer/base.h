#pragma once

#include <vector>
#include "System.h"

class Renderer : public System {
protected:
	uint16_t width = 1280;
	uint16_t height = 720;
	Decimal fovPlus = Decimal(glm::radians(10.0));
	Decimal zNear = Decimal(47.0 / 100000.0);
	Decimal zFar  = Decimal(48.0 / 100000.0);
	Decimal pixelDistanceFromScreen = Decimal(1000.0);
	virtual void MakePipeline(const std::vector<std::string> &) = 0;
public:
	bool showFPS = false;

	static bool IsSupported() { return false; }
	Renderer(Polar *engine) : System(engine) {}

	inline uint16_t GetWidth() { return width; }
	inline uint16_t GetHeight() { return height; }

	virtual void SetMouseCapture(bool) = 0;
	virtual void SetFullscreen(bool) = 0;
	virtual void SetPipeline(const std::vector<std::string> &) = 0;
	virtual void SetClearColor(const Point4 &) = 0;
	virtual Decimal GetUniformDecimal(const std::string &, const Decimal = 0) = 0;
	virtual Point3 GetUniformPoint3(const std::string &, const Point3 = Point3(0)) = 0;
	virtual void SetUniform(const std::string &, glm::uint32, bool = false) = 0;
	virtual void SetUniform(const std::string &, Decimal, bool = false) = 0;
	virtual void SetUniform(const std::string &, Point3, bool = false) = 0;
};
