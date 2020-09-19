#pragma once

#include <polar/component/base.h>

namespace polar::component {
	COMPONENT_BEGIN(window)
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

		static std::shared_ptr<window> deserialize(core::store_deserializer &s) {
			auto c = std::make_shared<window>();
			s >> c->size >> c->fullscreen >> c->capture_cursor;
			return c;
		}
	COMPONENT_END(window, window)
} // namespace polar::component
