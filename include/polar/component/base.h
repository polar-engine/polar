#pragma once

#include <polar/core/ecs.h>
#include <polar/core/types.h>
#include <polar/property/base.h>

namespace polar::component {
	class base : public core::ecs<property::base> {
	  public:
		virtual std::string name() const = 0;
	};
}
