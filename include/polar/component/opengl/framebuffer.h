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

	class double_framebuffer : public base {
	  public:
		std::array<GLuint, 2> fb;
		std::array<GLuint, 2> tex;

		size_t active = 0;

		double_framebuffer(GLuint fb0,
		                   GLuint fb1,
		                   GLuint tex0,
		                   GLuint tex1)
		  : fb({fb0, fb1}), tex({tex0, tex1}) {}

		virtual std::string name() const override { return "opengl_double_framebuffer"; }
	};
} // namespace polar::component::opengl
