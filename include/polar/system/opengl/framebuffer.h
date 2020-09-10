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
						auto fb0 = build_fb(win->size, comp->depth);
						auto fb1 = build_fb(win->size, comp->depth);
						component::opengl::framebuffer::fbs_type fbs = {fb0, fb1};
						engine->add<component::opengl::framebuffer>(wr, fbs);
					} else {
						auto fb = build_fb(win->size, comp->depth);
						engine->add<component::opengl::framebuffer>(wr, fb);
					}
					windows[comp->win].emplace(wr);
				}
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::framebuffer)) {
				auto comp = std::static_pointer_cast<component::opengl::framebuffer>(ptr.lock());

				for(auto &fb : comp->fbs) {
					GL(glDeleteTextures(fb.tex.size(), fb.tex.data()));
					GL(glDeleteFramebuffers(1, &fb.fb));
				}
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
					if(auto comp = engine->get<component::opengl::framebuffer>(fb_ref)) {
						for(auto &fb : comp->fbs) {
							for(auto &tex : fb.tex) {
								GL(glBindTexture(GL_TEXTURE_2D, tex));
								GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win->size.x, win->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
							}
						}
					}
				}
			}
		}

		GLuint build_tex(math::point2i size, bool depth = false) {
			GLuint texture;
			GL(glGenTextures(1, &texture));
			GL(glBindTexture(GL_TEXTURE_2D, texture));

			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

			if(depth) {
				GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL));
			} else {
				GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
			}

			return texture;
		}

		component::opengl::framebuffer::data_type build_fb(math::point2i size, bool depth) {
			GLuint fb;

			GL(glGenFramebuffers(1, &fb));
			GL(glBindFramebuffer(GL_FRAMEBUFFER, fb));

			std::vector<GLenum> draw_buffers = {GL_COLOR_ATTACHMENT0};
			if(depth) {
				draw_buffers.emplace_back(GL_DEPTH_ATTACHMENT);
			}

			std::vector<GLuint> tex;

			GLuint tex_color = build_tex(size);
			GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_color, 0));
			tex.emplace_back(tex_color);

			if(depth) {
				GLuint tex_depth = build_tex(size, depth);
				GL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex_depth, 0));
				tex.emplace_back(tex_depth);
			}

			GL(glDrawBuffers(draw_buffers.size(), draw_buffers.data()));

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

			return {fb, tex};
		}

	  public:
		static bool supported() { return true; }

		framebuffer(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "opengl_framebuffer"; }
	};
} // namespace polar::system::opengl
