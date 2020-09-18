#pragma once

#include <polar/component/base.h>
#include <polar/util/sdl.h>

namespace polar::component::opengl {
	class window : public base {
	  public:
		SDL_Window *win;
		SDL_GLContext ctx;

		window(SDL_Window *win, SDL_GLContext ctx) : win(win), ctx(ctx) {}

		bool serialize(core::store_serializer &s) const override {
			return false;
		}

		virtual std::string name() const override { return "opengl_window"; }
	};
} // namespace polar::component::opengl
