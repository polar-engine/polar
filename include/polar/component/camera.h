#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class camera : public base {
	  public:
		core::ref scene;
		core::ref target;
		math::mat4x4 projection;

		camera(core::ref scene,
		       core::ref target,
		       math::mat4x4 projection = math::mat4x4(1))
		  : scene(scene), target(target), projection(projection) {}

		virtual std::string name() const override { return "camera"; }
	};
} // namespace polar::component
