#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class camera : public base {
	  public:
		core::ref scene;
		core::ref target;

		camera(core::ref scene, core::ref target) : scene(scene), target(target) {}

		virtual std::string name() const override { return "camera"; }
	};
} // namespace polar::component
