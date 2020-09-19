#pragma once

#include <polar/component/base.h>
#include <polar/core/ref.h>

namespace polar::component {
	COMPONENT_BEGIN(framebuffer)
		core::ref win;
		bool depth = false;
		bool double_buffer = false;

		framebuffer(core::ref win, bool depth = false, bool double_buffer = false) : win(win), depth(depth), double_buffer(double_buffer) {}

		bool serialize(core::store_serializer &s) const override {
			s << win << depth << double_buffer;
			return true;
		}

		static std::shared_ptr<framebuffer> deserialize(core::store_deserializer &s) {
			core::ref win;
			s >> win;
			auto c = std::make_shared<framebuffer>(win);
			s >> c->depth >> c->double_buffer;
			return c;
		}
	COMPONENT_END(framebuffer, framebuffer)
} // namespace polar::component
