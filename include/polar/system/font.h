#pragma once

#include <polar/util/sdl.h>
#include <SDL_ttf.h>

namespace polar::system {
	class font : public base {
	  public:
		static bool supported() { return true; }

		font(core::polar *engine) : base(engine) {
			if(!SDL(TTF_Init())) { log()->fatal("font", "failed to init TTF"); }
		}

		~font() {
			SDL(TTF_Quit());
		}

		virtual std::string name() const override { return "font"; }
	};
} // namespace polar::system
