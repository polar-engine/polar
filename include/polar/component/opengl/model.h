#pragma once

#include <polar/component/base.h>
#include <polar/util/gl.h>

namespace polar::component::opengl {
	class model : public base {
	  public:
		GLuint vao;
		GLuint buffer;
		size_t count;

		model(GLuint vao, GLuint buffer, size_t count) : vao(vao), buffer(buffer), count(count) {}

		virtual std::string name() const override { return "opengl_model"; }
	};
} // namespace polar::component::opengl
