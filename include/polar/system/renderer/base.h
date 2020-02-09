#pragma once

#include <polar/system/action.h>
#include <polar/system/base.h>
#include <vector>

namespace polar::system::renderer {
	class base : public system::base {
	  protected:
		uint16_t width                  = 1280;
		uint16_t height                 = 720;
		math::decimal fovPlus           = math::decimal(glm::radians(10.0));

		virtual void makepipeline(const std::vector<std::string> &) = 0;

	  public:
		struct action_resize : action::digital {};

		math::decimal zNear                   = math::decimal(0.1);
		math::decimal zFar                    = math::decimal(1000.0);
		math::decimal pixelDistanceFromScreen = math::decimal(1000.0);

		bool showFPS = false;
		bool debug_draw = false;

		static bool supported() { return false; }
		base(core::polar *engine) : system::base(engine) {}

		virtual std::string name() const override { return "renderer"; }

		virtual accessor_list accessors() const override {
			accessor_list l;
			l.emplace_back("showfps", make_accessor<base>(
				[] (base *ptr) {
					return ptr->showFPS;
				},
				[] (base *ptr, auto x) {
					ptr->showFPS = x ? true : false;
				}
			));
			l.emplace_back("debugdraw", make_accessor<base>(
				[] (base *ptr) {
					return ptr->debug_draw;
				},
				[] (base *ptr, auto x) {
					ptr->debug_draw = x ? true : false;
				}
			));
			l.emplace_back("fullscreen", make_accessor<base>(
				[] (base *ptr) {
					return ptr->getfullscreen();
				},
				[] (base *ptr, auto x) {
					ptr->setfullscreen(x);
				}
			));
			l.emplace_back("width", make_accessor<base>(
				[] (base *ptr) {
					return ptr->getwidth();
				},
				[] (base *ptr, auto x) {
					ptr->setwidth(uint16_t(x));
				}
			));
			l.emplace_back("height", make_accessor<base>(
				[] (base *ptr) {
					return ptr->getheight();
				},
				[] (base *ptr, auto x) {
					ptr->setheight(uint16_t(x));
				}
			));
			return l;
		}

		inline uint16_t getwidth() { return width; }
		inline uint16_t getheight() { return height; }

		inline void setwidth(uint16_t w) { resize(w, height); }
		inline void setheight(uint16_t h) { resize(width, h); }

		virtual bool getfullscreen() = 0;

		virtual void resize(uint16_t, uint16_t)                                      = 0;
		virtual void setmousecapture(bool)                                           = 0;
		virtual void setfullscreen(bool)                                             = 0;
		virtual void setdepthtest(bool)                                              = 0;
		virtual void setpipeline(const std::vector<std::string> &)                   = 0;
		virtual void setclearcolor(const math::point4 &)                             = 0;
		virtual math::decimal getuniform_decimal(const std::string &,
		                                         const math::decimal = 0)            = 0;
		virtual math::point3 getuniform_point3(const std::string &,
		                                       const math::point3 = math::point3(0)) = 0;
		virtual void setuniform(const std::string &, glm::uint32,
		                        bool = false)                                        = 0;
		virtual void setuniform(const std::string &, math::decimal, bool = false)    = 0;
		virtual void setuniform(const std::string &, math::point3, bool = false)     = 0;
	};
} // namespace polar::system::renderer
