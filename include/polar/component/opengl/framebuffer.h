#pragma once

#include <polar/component/base.h>
#include <polar/util/gl.h>

namespace polar::component::opengl {
	class framebuffer : public base {
	  public:
		GLuint fb;
		GLuint tex;

		framebuffer(GLuint fb, GLuint tex) : fb(fb), tex(tex) {}

		virtual std::string name() const override { return "opengl_framebuffer"; }
	};
} // namespace polar::component::opengl
