#pragma once

#include <polar/system/renderer/base.h>
#include <polar/util/gl.h>
#include <polar/util/sdl.h>
#include <vector>

namespace polar::system::renderer {
	class gl32 : public base {
	  private:
		core::ref fps_object;

		void update(DeltaTicks &) override;

		void resize(uint16_t w, uint16_t h) override {
			width = w;
			height = h;

			// XXX: SDL(SDL_SetWindowSize(window, width, height));
			//rebuild();
		}

	  public:
		math::decimal fps = 60.0;

		static bool supported() { return true; }

		gl32(core::polar *engine) : base(engine) {}
	};
} // namespace polar::system::renderer
