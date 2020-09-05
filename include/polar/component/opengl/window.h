#pragma once

#include <polar/component/base.h>
#include <polar/util/sdl.h>

namespace polar::component::opengl {
	class window : public base {
	  public:
		SDL_Window *win;
		SDL_GLContext ctx;

		window(SDL_Window *win, SDL_GLContext ctx) : win(win), ctx(ctx) {}

		virtual std::string name() const override { return "opengl_window"; }
	};
} // namespace polar::component::opengl
