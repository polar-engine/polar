#pragma once

#include <polar/math/mat.h>
#include <polar/node/base.h>

namespace polar::node {
	struct perspective : base<math::mat4x4> {
		wrapped_node<math::point2>  size;
		wrapped_node<math::decimal> near;
		wrapped_node<math::decimal> far;

		perspective(decltype(size) size = math::point2(1280, 720),
		            decltype(near) near =    0.1f,
		            decltype(far)  far  = 1000.0f)
		  : size(size), near(near), far(far) {}

		math::mat4x4 eval(core::polar *engine) override {
			return math::perspective(size.eval(engine), near.eval(engine), far.eval(engine));
		}
	};
} // namespace polar::node
