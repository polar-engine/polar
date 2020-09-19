#pragma once

#include <polar/component/window.h>
#include <polar/node/base.h>

namespace polar::node::window {
	NODE_BEGIN(size, math::point2)
		core::ref window_ref;

		size(core::ref window_ref) : window_ref(window_ref) {}

		core::store_serializer & serialize(core::store_serializer &s) const override {
			return s << window_ref;
		}

		static size deserialize(core::store_deserializer &s) {
			core::ref win;
			s >> win;
			return size(win);
		}

		math::point2 eval(core::polar *engine) override {
			return engine->get<component::window>(window_ref)->size;
		}
	NODE_END(size, math::point2, window_size)
} // namespace polar::node
