#pragma once

#include <polar/component/window.h>
#include <polar/node/base.h>

namespace polar::node::window {
	struct size : base<math::point2> {
		core::ref window_ref;

		size(core::ref window_ref) : window_ref(window_ref) {}

		math::point2 eval(core::polar *engine) override {
			return engine->get<component::window>(window_ref)->size;
		}
	};
} // namespace polar::node
