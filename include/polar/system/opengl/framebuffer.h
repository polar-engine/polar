#pragma once

#include <polar/component/framebuffer.h>
#include <polar/component/opengl/framebuffer.h>
#include <polar/component/window.h>
#include <polar/system/base.h>

namespace polar::system::opengl {
	class framebuffer : public base {
	  private:
		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::framebuffer)) {
				auto comp = std::static_pointer_cast<component::framebuffer>(ptr.lock());

				if(auto win = engine->get<component::window>(comp->win)) {
					auto [fb, tex] = build_fb(win->size);
					engine->add<component::opengl::framebuffer>(wr, fb, tex);
				}
			}
		}

		void component_removed(core::weak_ref, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::framebuffer)) {
				auto comp = std::static_pointer_cast<component::opengl::framebuffer>(ptr.lock());

				GL(glDeleteTextures(1, &comp->tex));
				GL(glDeleteFramebuffers(1, &comp->fb));
			}
		}

		GLuint build_tex(math::point2i size) {
			GLuint texture;
			GL(glGenTextures(1, &texture));
			GL(glBindTexture(GL_TEXTURE_2D, texture));

			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

			GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

			return texture;
		}

		std::pair<GLuint, GLuint> build_fb(math::point2i size) {
			GLuint fb;
			GL(glGenFramebuffers(1, &fb));
			GL(glBindFramebuffer(GL_FRAMEBUFFER, fb));

			GLenum draw_buffer = GL_COLOR_ATTACHMENT0;

			GLuint texture = build_tex(size);

			GL(glFramebufferTexture(GL_FRAMEBUFFER, draw_buffer, texture, 0));
			GL(glDrawBuffers(1, &draw_buffer));

			GLenum status;
			GL(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

			std::stringstream msg;
			switch(status) {
			case GL_FRAMEBUFFER_COMPLETE:
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				log()->fatal("opengl::framebuffer", "framebuffer unsupported");
			default:
				msg << "framebuffer status incomplete (0x" << std::hex << status << ')';
				log()->fatal("opengl::framebuffer", msg.str());
			}

			return {fb, texture};
		}

	  public:
		static bool supported() { return true; }

		framebuffer(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "opengl_framebuffer"; }
	};
} // namespace polar::system::opengl
