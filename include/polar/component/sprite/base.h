#pragma once

#include <polar/component/base.h>
#include <polar/util/sdl.h>

namespace polar::component::sprite {
	class base : public component::base {
	  public:
		SDL_Surface *surface = nullptr;
		bool freeSurface     = true;

		virtual ~base() {
			if(surface && freeSurface) { SDL(SDL_FreeSurface(surface)); }
		}

		void render() {
			if(surface && freeSurface) { SDL(SDL_FreeSurface(surface)); }
			render_me();
		}

		virtual void render_me() = 0;
	};
} // namespace polar::component::sprite
