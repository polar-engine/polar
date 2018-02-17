#pragma once

#include <polar/system/action.h>
#include <polar/system/base.h>
#include <vector>

namespace polar::system::renderer {
	class base : public system::base {
	  protected:
		uint16_t width                  = 1280;
		uint16_t height                 = 720;
		Decimal fovPlus                 = Decimal(glm::radians(10.0));
		Decimal zNear                   = Decimal(47.0 / 100000.0);
		Decimal zFar                    = Decimal(48.0 / 100000.0);
		Decimal pixelDistanceFromScreen = Decimal(1000.0);
		virtual void makepipeline(const std::vector<std::string> &) = 0;

	  public:
		struct action_resize : action::digital {};

		bool showFPS = false;

		static bool supported() { return false; }
		base(core::polar *engine) : system::base(engine) {}

		inline uint16_t getwidth() { return width; }
		inline uint16_t getheight() { return height; }

		virtual void setmousecapture(bool)                                  = 0;
		virtual void setfullscreen(bool)                                    = 0;
		virtual void setpipeline(const std::vector<std::string> &)          = 0;
		virtual void setclearcolor(const Point4 &)                          = 0;
		virtual Decimal getuniform_decimal(const std::string &,
		                                   const Decimal = 0)               = 0;
		virtual Point3 getuniform_point3(const std::string &,
		                                 const Point3 = Point3(0))          = 0;
		virtual void setuniform(const std::string &, glm::uint32,
		                        bool = false)                               = 0;
		virtual void setuniform(const std::string &, Decimal, bool = false) = 0;
		virtual void setuniform(const std::string &, Point3, bool = false)  = 0;
	};
} // namespace polar::system::renderer
