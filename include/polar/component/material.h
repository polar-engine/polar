#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class material : public base {
	  public:
		core::ref diffuse;

		material(core::ref diffuse) : diffuse(diffuse) {}

		virtual std::string name() const override { return "material"; }
	};
} // namespace polar::component
