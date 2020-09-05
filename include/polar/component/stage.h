#pragma once

#include <polar/component/base.h>
#include <polar/core/ref.h>

namespace polar::component {
	class stage : public base {
	  public:
		std::string source;
		core::ref target;

		stage(std::string source, core::ref target) : source(source), target(target) {}

		virtual std::string name() const override { return "stage"; }
	};
} // namespace polar::component
