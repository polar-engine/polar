#pragma once

#include <polar/component/opengl/texture.h>
#include <polar/component/texture.h>
#include <polar/system/base.h>

namespace polar::system::opengl {
	class texture : public base {
	  private:
		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) {
			if(ti == typeid(component::texture)) {
				auto comp = std::static_pointer_cast<component::texture>(ptr.lock());

				GLuint tex;
				GL(glGenTextures(1, &tex));
				GL(glBindTexture(GL_TEXTURE_2D, tex));

				GLint format = GL_RGBA;
				GL(glTexImage2D(GL_TEXTURE_2D, 0, format, comp->width, comp->height, 0, format,
				                GL_UNSIGNED_BYTE, comp->pixels.data()));
				GL(glGenerateMipmap(GL_TEXTURE_2D));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));

				engine->add<component::opengl::texture>(wr, tex);
			}
		}

		void component_removed(core::weak_ref, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::texture)) {
				auto comp = std::static_pointer_cast<component::opengl::texture>(ptr.lock());

				GL(glDeleteTextures(1, &comp->tex));
			}
		}

	  public:
		static bool supported() { return true; }

		texture(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "opengl_texture"; }
	};
} // namespace polar::system::opengl
