#pragma once

#include <polar/component/base.h>
#include <polar/util/gl.h>

namespace polar::component::opengl {
	class texture : public base {
	  public:
		GLuint tex;

		texture(GLuint tex) : tex(tex) {}

		bool serialize(core::store_serializer &s) const override {
			return false;
		}

		virtual std::string name() const override { return "opengl_texture"; }
	};
} // namespace polar::component::opengl
