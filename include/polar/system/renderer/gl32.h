#pragma once

#include <polar/system/renderer/base.h>
#include <polar/util/gl.h>
#include <polar/util/sdl.h>
#include <vector>

namespace polar::system::renderer {
	class gl32 : public base {

	  private:
		std::vector<glm::vec3> debug_box_points;
		std::vector<glm::vec3> debug_ball_points;

		GLuint debug_box_vao;
		GLuint debug_ball_vao;

		core::ref fps_object;

		void update(DeltaTicks &) override;

		math::mat4x4 calculate_projection();

		void resize(uint16_t w, uint16_t h) override {
			width = w;
			height = h;

			// XXX: SDL(SDL_SetWindowSize(window, width, height));
			//rebuild();
		}

		/*
		void rebuild() {
			auto act = engine->get<action>().lock();
			GL(glViewport(0, 0, width, height));
			//makepipeline(pipelineNames);
			if(act) {
				act->trigger<action_resize>();
			}
		}
		*/

	  public:
		math::decimal fps = 60.0;

		static bool supported() { return true; }

		gl32(core::polar *engine) : base(engine) {}
	};
} // namespace polar::system::renderer
