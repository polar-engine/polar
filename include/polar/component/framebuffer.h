#pragma once

#include <polar/component/base.h>
#include <polar/core/ref.h>

namespace polar::component {
	class framebuffer : public base {
	  public:
		core::ref win;
		bool double_buffer = false;

		framebuffer(core::ref win, bool double_buffer = false) : win(win), double_buffer(double_buffer) {}

		virtual std::string name() const override { return "framebuffer"; }
	};
} // namespace polar::component
