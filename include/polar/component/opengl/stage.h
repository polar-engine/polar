#pragma once

#include <polar/component/base.h>

namespace polar::component::opengl {
	class stage : public base {
	  public:
		GLuint program;

		stage(GLuint program) : program(program) {}

		bool serialize(core::store_serializer &s) const override {
			return false;
		}

		virtual std::string name() const override { return "opengl_stage"; }
	};
} // namespace polar::component::opengl
