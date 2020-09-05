#pragma once

#include <polar/component/window.h>
#include <polar/component/opengl/window.h>
#include <polar/support/action/controller.h>
#include <polar/support/action/keyboard.h>
#include <polar/support/action/mouse.h>
#include <polar/system/base.h>
#include <polar/system/vr.h>
#include <polar/util/sdl.h>

namespace polar::system::opengl {
	class window : public base {
	  private:
		void update(DeltaTicks &dt) override {
			SDL_Event event;
			while(SDL_PollEvent(&event)) { handle_event(event); }
			SDL_ClearError();

			auto ti_range = engine->objects.get<core::index::ti>().equal_range(typeid(component::opengl::window));
			for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
				auto comp = std::static_pointer_cast<component::opengl::window>(ti_it->ptr);
			}
		}

		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::window)) {
				auto comp = std::static_pointer_cast<component::window>(ptr.lock());

				if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) {
					log()->fatal("opengl::window", "failed to set major version attribute");
				}
				if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1))) {
					log()->fatal("opengl::window", "failed to set minor version attribute");
				}
				if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) {
					log()->fatal("opengl::window", "failed to set profile mask attribute");
				}
				if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) {
					log()->fatal("opengl::window", "failed to set double buffer attribute");
				}
				if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG))) {
					log()->fatal("opengl::window", "failed to set context flags");
				}

				SDL_Window *win;

				Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
				if(!SDL(win = SDL_CreateWindow("Polar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				                                  comp->size.x, comp->size.y, window_flags))) {
					log()->fatal("opengl::window", "failed to create window");
				}

				SDL_GLContext ctx;
				if(!SDL(ctx = SDL_GL_CreateContext(win))) { log()->fatal("opengl::window", "failed to create OpenGL context"); }

				if(!SDL(SDL_GL_SetSwapInterval(1))) { log()->critical("opengl::window", "failed to set swap interval"); }

				glewExperimental = GL_TRUE;
				GLenum err       = glewInit();

				if(err != GLEW_OK) { log()->fatal("opengl::window", "GLEW: glewInit failed"); }

				// GLEW calls glGetString(EXTENSIONS) which causes GL_INVALID_ENUM on core contexts
				glGetError();

				auto &col = comp->clear_color;
				GL(glClearColor(col.r, col.g, col.b, col.a));

				GL(glEnable(GL_DEPTH_TEST));
				GL(glEnable(GL_BLEND));
				GL(glEnable(GL_CULL_FACE));
				GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
				GL(glCullFace(GL_BACK));

				if(!SDL(SDL_SetWindowFullscreen(win, comp->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))) {
					log()->critical("opengl::window", "failed to set fullscreen mode");
				}

				if(!SDL(SDL_SetRelativeMouseMode(comp->capture_cursor ? SDL_TRUE : SDL_FALSE))) {
					log()->fatal("opengl::window", "failed to set relative mouse mode");
				}

				engine->add<component::opengl::window>(wr, win, ctx);
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::window)) {
				auto comp = std::static_pointer_cast<component::opengl::window>(ptr.lock());
				SDL(SDL_GL_DeleteContext(comp->ctx));
				SDL(SDL_DestroyWindow(comp->win));
			}
		}

		void handle_event(SDL_Event &ev) {
			namespace kb         = support::action::keyboard;
			namespace mouse      = support::action::mouse;
			namespace controller = support::action::controller;

			support::input::key key;

			auto act = engine->get<action>().lock();
			auto vr  = engine->get<system::vr>().lock();

			switch(ev.type) {
			case SDL_QUIT:
				engine->quit();
				break;
			case SDL_WINDOWEVENT:
				switch(ev.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					//GL(glViewport(0, 0, ev.window.data1, ev.window.data2));

					/* XXX
					if(vr && vr->ready()) {
						width  = vr->width();
						height = vr->height();
					} else {
						int w, h;
						SDL(SDL_GL_GetDrawableSize(SDL_GetWindowFromID(ev.window.windowID), &w, &h));
						width  = w;
						height = h;
					}

					rebuild();
					*/
					break;
				}
				break;
			case SDL_KEYDOWN:
				if(ev.key.repeat == 0) {
					key = mkKeyFromSDL(ev.key.keysym.sym);
					if(act) { act->trigger_digital(kb::key_ti(key), true); }
				}
				break;
			case SDL_KEYUP:
				key = mkKeyFromSDL(ev.key.keysym.sym);
				if(act) { act->trigger_digital(kb::key_ti(key), false); }
				break;
			case SDL_MOUSEBUTTONDOWN:
				key = mkMouseButtonFromSDL(ev.button.button);
				if(act) { act->trigger_digital(kb::key_ti(key), true); }
				break;
			case SDL_MOUSEBUTTONUP:
				key = mkMouseButtonFromSDL(ev.button.button);
				if(act) { act->trigger_digital(kb::key_ti(key), false); }
				break;
			case SDL_MOUSEMOTION:
				if(act) {
					// XXX: this is hacky
					act->accumulate<mouse::position_x>(math::decimal(ev.motion.x));
					act->accumulate<mouse::position_y>(math::decimal(ev.motion.y));

					act->accumulate<mouse::motion_x>(math::decimal(ev.motion.xrel));
					act->accumulate<mouse::motion_y>(math::decimal(ev.motion.yrel));
				}
				break;
			case SDL_MOUSEWHEEL:
				if(act) {
					act->accumulate<mouse::wheel_x>(math::decimal(ev.wheel.x));
					act->accumulate<mouse::wheel_y>(math::decimal(ev.wheel.y));
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				key = mkButtonFromSDL(static_cast<SDL_GameControllerButton>(ev.cbutton.button));
				if(act) { act->trigger_digital(kb::key_ti(key), true); }
				break;
			case SDL_CONTROLLERBUTTONUP:
				key = mkButtonFromSDL(static_cast<SDL_GameControllerButton>(ev.cbutton.button));
				if(act) { act->trigger_digital(kb::key_ti(key), false); }
				break;
			case SDL_CONTROLLERAXISMOTION:
				switch(ev.caxis.axis) {
				case 0: // x axis
					if(act) { act->accumulate<controller::motion_x>(ev.caxis.value); }
					break;
				case 1: // y axis
					if(act) { act->accumulate<controller::motion_y>(ev.caxis.value); }
					break;
				}
				break;
			}
		}

	  public:
		static bool supported() { return true; }

		window(core::polar *engine) : base(engine) {
			if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) { log()->fatal("opengl::window", "failed to init SDL"); }

			// set up controller joysticks
			SDL_GameController *controller = nullptr;
			for(int i = 0; i < SDL_NumJoysticks(); ++i) {
				bool is_controller;
				SDL(is_controller = SDL_IsGameController(i));
				if(is_controller) {
					log()->verbose("opengl::window", "SDL detected controller #", i);
					SDL(controller = SDL_GameControllerOpen(i));
				}
			}
			SDL(SDL_GameControllerEventState(SDL_ENABLE));
		}

		~window() {
			SDL(SDL_GL_ResetAttributes());
			SDL(SDL_Quit());
		}

		virtual std::string name() const override { return "opengl_window"; }
	};
} // namespace polar::system::opengl
