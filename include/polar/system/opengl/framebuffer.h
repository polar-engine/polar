#pragma once

#include <polar/component/framebuffer.h>
#include <polar/component/opengl/framebuffer.h>
#include <polar/component/window.h>
#include <polar/system/base.h>

namespace polar::system::opengl {
	class framebuffer : public base {
	  private:
		using fbs_type = std::set<core::weak_ref>;
		std::map<core::weak_ref, fbs_type> windows;

		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::framebuffer)) {
				auto comp = std::static_pointer_cast<component::framebuffer>(ptr.lock());

				if(auto win = engine->get<component::window>(comp->win)) {
					if(comp->double_buffer) {
						auto [fb0, tex0] = build_fb(win->size);
						auto [fb1, tex1] = build_fb(win->size);
						engine->add<component::opengl::double_framebuffer>(wr, fb0, fb1, tex0, tex1);
					} else {
						auto [fb, tex] = build_fb(win->size);
						engine->add<component::opengl::framebuffer>(wr, fb, tex);
					}
					windows[comp->win].emplace(wr);
				}
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::framebuffer)) {
				auto comp = std::static_pointer_cast<component::opengl::framebuffer>(ptr.lock());

				GL(glDeleteTextures(1, &comp->tex));
				GL(glDeleteFramebuffers(1, &comp->fb));
			} else if(ti == typeid(component::opengl::double_framebuffer)) {
				auto comp = std::static_pointer_cast<component::opengl::double_framebuffer>(ptr.lock());

				GL(glDeleteTextures(comp->tex.size(), comp->tex.data()));
				GL(glDeleteFramebuffers(comp->fb.size(), comp->fb.data()));
			} else if(ti == typeid(component::framebuffer)) {
				auto comp = std::static_pointer_cast<component::framebuffer>(ptr.lock());

				auto search = windows.find(comp->win);
				if(search != windows.end()) {
					search->second.erase(wr);
				}
			}
		}

		void mutate(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::window)) {
				auto win = std::static_pointer_cast<component::window>(ptr.lock());

				for(auto fb_ref : windows[wr]) {
					log()->debug("opengl::framebuffer", "win mutated, updating fb!");
					if(auto fb = engine->get<component::opengl::framebuffer>(fb_ref)) {
						GL(glBindTexture(GL_TEXTURE_2D, fb->tex));
						GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win->size.x, win->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
					}
					if(auto fb = engine->get<component::opengl::double_framebuffer>(fb_ref)) {
						for(auto tex : fb->tex) {
							GL(glBindTexture(GL_TEXTURE_2D, tex));
							GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win->size.x, win->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
						}
					}
				}
			}
		}

		GLuint build_tex(math::point2i size) {
			GLuint texture;
			GL(glGenTextures(1, &texture));
			GL(glBindTexture(GL_TEXTURE_2D, texture));

			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

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
