#pragma once

#include <array>
#include <boost/container/flat_set.hpp>
#include <functional>
#include <polar/asset/font.h>
#include <polar/asset/shaderprogram.h>
#include <polar/component/model.h>
#include <polar/component/phys.h>
#include <polar/component/sprite/base.h>
#include <polar/component/text.h>
#include <polar/property/gl32/model.h>
#include <polar/property/gl32/sprite.h>
#include <polar/support/gl32/fontcache.h>
#include <polar/support/gl32/pipelinenode.h>
#include <polar/system/renderer/base.h>
#include <polar/util/gl.h>
#include <polar/util/sdl.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace polar::system::renderer {
	class gl32 : public base {
		using pipelinenode = support::gl32::pipelinenode;
		using fontcache_t  = support::gl32::fontcache;
		using model_p      = property::gl32::model;
		using sprite_p     = property::gl32::sprite;

	  private:
		bool inited     = false;
		bool capture    = false;
		bool fullscreen = false;

		SDL_Window *window = nullptr;
		SDL_GLContext context;

		std::vector<std::string> pipelineNames;
		std::vector<pipelinenode> nodes;
		std::unordered_multiset<std::shared_ptr<model_p>> modelPropertyPool;
		std::unordered_map<std::shared_ptr<polar::asset::font>, fontcache_t>
		    fontCache;

		std::vector<glm::vec2> viewportPoints;
		std::vector<glm::vec3> debug_box_points;
		std::vector<glm::vec3> debug_ball_points;

		GLuint viewportVAO;
		GLuint debug_box_vao;
		GLuint debug_ball_vao;

		GLuint spriteProgram;
		GLuint identityProgram;
		GLuint debugProgram;
		GLuint ditherTex;

		core::ref fps_object;

		std::unordered_map<std::string, glm::uint32> uniformsU32;
		std::unordered_map<std::string, math::decimal> uniformsFloat;
		std::unordered_map<std::string, math::point3> uniformsPoint3;

		std::vector<std::string> changedUniformsU32;
		std::vector<std::string> changedUniformsFloat;
		std::vector<std::string> changedUniformsPoint3;

		void init() override;
		void update(DeltaTicks &) override;
		void rendersprite(core::weak_ref, math::mat4x4 = math::mat4x4(1), math::mat4x4 view = math::mat4x4(1));
		void rendertext(core::weak_ref, math::mat4x4 proj = math::mat4x4(1), math::mat4x4 view = math::mat4x4(1));
		void render(math::mat4x4 proj, math::mat4x4 view, float delta);

		std::shared_ptr<model_p> getpooledmodelproperty(const GLsizei required);

		void uploadmodel(std::shared_ptr<component::model> model);

		void component_added(core::weak_ref, std::type_index, std::weak_ptr<component::base>) override;
		void component_removed(core::weak_ref, std::type_index, std::weak_ptr<component::base>) override;

		math::mat4x4 calculate_projection();
		void project(GLuint programID, math::mat4x4 proj);
		inline void project(GLuint programID) { project(programID, calculate_projection()); }

		void initGL();
		void handleSDL(SDL_Event &);
		void makepipeline(const std::vector<std::string> &) override;
		GLuint makeprogram(std::shared_ptr<polar::asset::shaderprogram>);

	  public:
		math::decimal fps = 60.0;

		static bool supported();
		gl32(core::polar *engine, const std::vector<std::string> &names)
		    : base(engine) {
			setpipeline(names);
		}
		~gl32();

		void resize(uint16_t w, uint16_t h) override {
			width = w;
			height = h;

			SDL(SDL_SetWindowSize(window, width, height));
			rebuild();
		}

		void rebuild() {
			auto act = engine->get<action>().lock();
			GL(glViewport(0, 0, width, height));
			makepipeline(pipelineNames);
			if(act) {
				act->trigger<action_resize>();
			}
		}

		bool getfullscreen() override {
			return fullscreen;
		}

		void setmousecapture(bool capture) override;
		void setfullscreen(bool fullscreen) override;
		void setdepthtest(bool depthtest) override;
		void setpipeline(const std::vector<std::string> &names) override;
		void setclearcolor(const math::point4 &color) override;

		math::decimal getuniform_decimal(const std::string &name, const math::decimal def) override;
		math::point3 getuniform_point3(const std::string &name, const math::point3 def) override;

		void setuniform(const std::string &name, glm::uint32 x, bool force = false) override;
		void setuniform(const std::string &name, math::decimal x, bool force = false) override;
		void setuniform(const std::string &name, math::point3 p, bool force = false) override;

		bool uploaduniform(GLuint program, const std::string &name, glm::int32 x);
		bool uploaduniform(GLuint program, const std::string &name, glm::uint32 x);
		bool uploaduniform(GLuint program, const std::string &name, math::decimal x);
		bool uploaduniform(GLuint program, const std::string &name, math::point2 p);
		bool uploaduniform(GLuint program, const std::string &name, math::point3 p);
		bool uploaduniform(GLuint program, const std::string &name, math::point4 p);
		bool uploaduniform(GLuint program, const std::string &name, math::mat4x4 m);
	};
} // namespace polar::system::renderer
