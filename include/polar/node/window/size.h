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

		static size deserialize(core::deserializer &s) {
			throw std::runtime_error("not implemented");
			//size n;
			//s >> n.window_ref; XXX
			//return n;
		}

		math::point2 eval(core::polar *engine) override {
			return engine->get<component::window>(window_ref)->size;
		}
	NODE_END(size, math::point2, "window.size")
} // namespace polar::node
