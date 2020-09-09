#pragma once

#include <polar/component/base.h>
#include <polar/core/ref.h>

namespace polar::component {
	class stage : public base {
	  public:
		std::string source;

		stage(std::string source) : source(source) {}

		virtual std::string name() const override { return "stage"; }
	};
} // namespace polar::component
