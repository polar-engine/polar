#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class window : public base {
	  public:
		math::point2 size;
		math::point4 clear_color;
		bool fullscreen;
		bool capture_cursor;

		window(math::point2i size       = math::point2i(1280, 720),
		       math::point4 clear_color = math::point4(0, 0, 0, 0),
		       bool fullscreen          = false,
		       bool capture_cursor      = false)
		  : size(size), clear_color(clear_color), fullscreen(fullscreen), capture_cursor(capture_cursor) {}

		virtual std::string name() const override { return "window"; }
	};
} // namespace polar::component
