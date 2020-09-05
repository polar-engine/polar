#pragma once

#include <polar/component/base.h>

namespace polar::component::opengl {
	class stage : public base {
	  public:
		GLuint program;

		stage(GLuint program) : program(program) {}

		virtual std::string name() const override { return "opengl_stage"; }
	};
} // namespace polar::component::opengl
