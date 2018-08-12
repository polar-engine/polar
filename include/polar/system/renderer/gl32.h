#pragma once

#include <array>
#include <boost/container/flat_set.hpp>
#include <functional>
#include <polar/asset/font.h>
#include <polar/asset/shaderprogram.h>
#include <polar/component/model.h>
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

		GLuint viewportVAO;
		GLuint spriteProgram;
		GLuint identityProgram;

		core::ref fpsDtor;
		IDType fpsID = 0;

		std::unordered_map<std::string, glm::uint32> uniformsU32;
		std::unordered_map<std::string, Decimal> uniformsFloat;
		std::unordered_map<std::string, Point3> uniformsPoint3;

		std::vector<std::string> changedUniformsU32;
		std::vector<std::string> changedUniformsFloat;
		std::vector<std::string> changedUniformsPoint3;

		void init() override;
		void update(DeltaTicks &) override;
		void rendersprite(IDType, Mat4 = Mat4(1));
		void rendertext(IDType, Mat4 = Mat4(1));
		void render(Mat4 proj, Mat4 view, float alpha);

		std::shared_ptr<model_p> getpooledmodelproperty(const GLsizei required);

		void uploadmodel(std::shared_ptr<component::model> model);

		void componentadded(IDType id, std::type_index ti,
		                    std::weak_ptr<component::base> ptr) override;
		void componentremoved(IDType id, std::type_index ti) override;

		Mat4 calculate_projection();
		void project(GLuint programID, Mat4 proj);
		inline void project(GLuint programID) { project(programID, calculate_projection()); }

		void initGL();
		void handleSDL(SDL_Event &);
		void makepipeline(const std::vector<std::string> &) override;
		GLuint makeprogram(std::shared_ptr<polar::asset::shaderprogram>);

	  public:
		Decimal fps = 60.0;

		static bool supported();
		gl32(core::polar *engine, const std::vector<std::string> &names)
		    : base(engine) {
			setpipeline(names);
		}
		~gl32();

		void setmousecapture(bool capture) override;
		void setfullscreen(bool fullscreen) override;
		void setpipeline(const std::vector<std::string> &names) override;
		void setclearcolor(const Point4 &color) override;

		Decimal getuniform_decimal(const std::string &name,
		                           const Decimal def) override;
		Point3 getuniform_point3(const std::string &name,
		                         const Point3 def) override;

		void setuniform(const std::string &name, glm::uint32 x,
		                bool force = false) override;
		void setuniform(const std::string &name, Decimal x,
		                bool force = false) override;
		void setuniform(const std::string &name, Point3 p,
		                bool force = false) override;

		bool uploaduniform(GLuint program, const std::string &name,
		                   glm::int32 x);
		bool uploaduniform(GLuint program, const std::string &name,
		                   glm::uint32 x);
		bool uploaduniform(GLuint program, const std::string &name, Decimal x);
		bool uploaduniform(GLuint program, const std::string &name, Point2 p);
		bool uploaduniform(GLuint program, const std::string &name, Point3 p);
		bool uploaduniform(GLuint program, const std::string &name, Point4 p);
		bool uploaduniform(GLuint program, const std::string &name, Mat4 m);
	};
} // namespace polar::system::renderer
