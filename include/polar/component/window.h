#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class window : public base {
	  public:
		math::point2 size;
		bool fullscreen;
		bool capture_cursor;

		window(math::point2i size  = math::point2i(1280, 720),
		       bool fullscreen     = false,
		       bool capture_cursor = false)
		  : size(size), fullscreen(fullscreen), capture_cursor(capture_cursor) {}

		bool serialize(core::store_serializer &s) const override {
			s << size << fullscreen << capture_cursor;
			return true;
		}

		virtual std::string name() const override { return "window"; }
	};
} // namespace polar::component
