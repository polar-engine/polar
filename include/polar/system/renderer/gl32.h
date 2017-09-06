#pragma once

#include <functional>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <boost/container/flat_set.hpp>
#include <polar/system/renderer/base.h>
#include <polar/asset/font.h>
#include <polar/asset/shaderprogram.h>
#include <polar/component/model.h>
#include <polar/component/text.h>
#include <polar/component/sprite/base.h>
#include <polar/property/gl32/model.h>
#include <polar/property/gl32/sprite.h>
#include <polar/support/gl32/pipelinenode.h>
#include <polar/support/gl32/fontcache.h>
#include <polar/util/sharedptr_less.h>
#include <polar/util/sdl.h>
#include <polar/util/gl.h>

namespace polar { namespace system { namespace renderer {
	class gl32 : public base {
		using pipelinenode = support::gl32::pipelinenode;
		using fontcache_t = support::gl32::fontcache;
		using model_p = property::gl32::model;
		using sprite_p = property::gl32::sprite;
	private:
		bool inited = false;
		bool capture = false;
		bool fullscreen = false;

		SDL_Window *window;
		SDL_GLContext context;
		std::vector<std::string> pipelineNames;
		std::vector<pipelinenode> nodes;
		boost::container::flat_multiset<std::shared_ptr<model_p>, sharedptr_less<model_p>> modelPropertyPool;
		std::unordered_map<std::shared_ptr<polar::asset::font>, fontcache_t> fontCache;

		GLuint viewportVAO;
		GLuint spriteProgram;

		std::shared_ptr<core::destructor> fpsDtor;
		IDType fpsID = 0;

		std::unordered_map<std::string, glm::uint32> uniformsU32;
		std::unordered_map<std::string, Decimal> uniformsFloat;
		std::unordered_map<std::string, Point3> uniformsPoint3;

		std::vector<std::string> changedUniformsU32;
		std::vector<std::string> changedUniformsFloat;
		std::vector<std::string> changedUniformsPoint3;

		void init() override final;
		void update(DeltaTicks &) override final;
		void rendersprite(IDType);
		void rendertext(IDType);

		inline std::shared_ptr<model_p> getpooledmodelproperty(const GLsizei required) {
			if(modelPropertyPool.empty()) {
				model_p prop;

				GL(glGenVertexArrays(1, &prop.vao));
				GL(glBindVertexArray(prop.vao));

				/* location   attribute
				 *
				 *        0   vertex
				 *        1   normal
				 */

				prop.vbos.resize(2);
				GL(glGenBuffers(2, &prop.vbos[0]));

				GL(glBindBuffer(GL_ARRAY_BUFFER, prop.vbos[0]));
				GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));

				GL(glBindBuffer(GL_ARRAY_BUFFER, prop.vbos[1]));
				GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL));

				GL(glEnableVertexAttribArray(0));
				GL(glEnableVertexAttribArray(1));

				return std::make_shared<model_p>(prop);
			} else {
				auto dummy = std::make_shared<model_p>();
				dummy->capacity = required;

				auto it = modelPropertyPool.lower_bound(dummy);
				if(it == modelPropertyPool.cend()) { it = modelPropertyPool.begin(); }

				auto prop = *it;
				modelPropertyPool.erase(it);
				return prop;
			}
		}

		inline void uploadmodel(std::shared_ptr<component::model> model) {
			auto normals = model->calculate_normals();
			auto numVertices = normals.size();
			auto dataSize = sizeof(component::model::PointType) * numVertices;
			auto prop = getpooledmodelproperty(numVertices);

			if(numVertices > 0) {
				if(GLsizei(numVertices) > prop->capacity) {
					GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
					GL(glBufferData(GL_ARRAY_BUFFER, dataSize, model->points.data(), GL_DYNAMIC_DRAW));

					GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
					GL(glBufferData(GL_ARRAY_BUFFER, dataSize, normals.data(), GL_DYNAMIC_DRAW));

					prop->capacity = numVertices;
				} else {
					GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
					GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, model->points.data()));

					GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
					GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, normals.data()));
				}
			}

			prop->numVertices = numVertices;

			model->add<model_p>(prop);
			//model->points.clear();
			//model->points.shrink_to_fit();
		}

		inline void componentadded(IDType id, const std::type_info *ti, std::weak_ptr<component::base> ptr) override final {
			if(ti == &typeid(component::model)) {
				auto model = std::static_pointer_cast<component::model>(ptr.lock());
				uploadmodel(model);
			} else if(ti == &typeid(component::sprite::base)) {
				auto sprite = std::static_pointer_cast<component::sprite::base>(ptr.lock());
				sprite->render();
				sprite_p prop;

				GL(glGenTextures(1, &prop.texture));
				GL(glBindTexture(GL_TEXTURE_2D, prop.texture));

				GLint format = GL_RGBA;
				GL(glTexImage2D(GL_TEXTURE_2D, 0, format, sprite->surface->w, sprite->surface->h, 0, format, GL_UNSIGNED_BYTE, sprite->surface->pixels));
				GL(glGenerateMipmap(GL_TEXTURE_2D));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));

				sprite->add<sprite_p>(prop);
			} else if(ti == &typeid(component::text)) {
				auto text = std::static_pointer_cast<component::text>(ptr.lock());
				auto &cache = fontCache.emplace(text->as, fontcache_t()).first->second;

				for(auto c : text->str) {
					auto &entry = cache.entries[c];

					if(!entry.active) {
						auto &glyph = text->as->glyphs[c];

						GL(glGenTextures(1, &entry.texture));
						GL(glBindTexture(GL_TEXTURE_2D, entry.texture));

						GLint format = GL_RGBA;
						GL(glTexImage2D(GL_TEXTURE_2D, 0, format, glyph.surface->w, glyph.surface->h, 0, format, GL_UNSIGNED_BYTE, glyph.surface->pixels));
						GL(glGenerateMipmap(GL_TEXTURE_2D));
						GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
						GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
						GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
						GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));

						entry.active = true;
					}
				}
			}
		}

		inline void componentremoved(IDType id, const std::type_info *ti) override final {
			if(ti == &typeid(component::model)) {
				auto model = engine->getcomponent<component::model>(id);
				if(model != nullptr) {
					auto prop = model->get<model_p>().lock();
					if(prop) {
						modelPropertyPool.emplace(prop);
					}
				}
			} else if(ti == &typeid(component::sprite::base)) {
				auto text = engine->getcomponent<component::sprite::base>(id);
				if(text != nullptr) {
					auto prop = text->get<sprite_p>().lock();
					if(prop) {
						GL(glDeleteTextures(1, &prop->texture));
					}
				}
			}
		}

		inline Mat4 CalculateProjection() {
			auto heightF = static_cast<Decimal>(height);
			auto fovy = 2.0f * glm::atan(heightF, Decimal(2) * pixelDistanceFromScreen) + fovPlus;
			auto projection = glm::perspective(fovy, static_cast<Decimal>(width) / heightF, zNear, zFar);
			//auto projection = glm::infinitePerspective(fovy, static_cast<Decimal>(width) / heightF, zNear);
			return projection;
		}

		inline void project(GLuint programID) {
			GL(glUseProgram(programID));
			uploaduniform(programID, "u_projection", CalculateProjection());
		}

		void initGL();
		void handleSDL(SDL_Event &);
		void makepipeline(const std::vector<std::string> &) override final;
		GLuint makeprogram(std::shared_ptr<polar::asset::shaderprogram>);
	public:
		Decimal fps = 60.0;

		static bool supported();
		gl32(core::polar *engine, const std::vector<std::string> &names) : base(engine), pipelineNames(names) {}
		~gl32();

		void setmousecapture(bool capture) override final {
			this->capture = capture;
			if(inited) {
				if(!SDL(SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE))) { debugmanager()->fatal("failed to set relative mouse mode"); }
			}
		}

		void setfullscreen(bool fullscreen) override final {
			this->fullscreen = fullscreen;
			if(inited) {
				if(!SDL(SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))) { debugmanager()->critical("failed to set fullscreen mode"); }
			}
		}

		void setpipeline(const std::vector<std::string> &names) override final {
			pipelineNames = names;
			if(inited) {
				debugmanager()->trace("MakePipeline from SetPipeline");
				makepipeline(names);
				debugmanager()->trace("MakePipeline done");
			}
		}

		inline void setclearcolor(const Point4 &color) override final {
			auto color2 = glm::vec4(color);
			GL(glClearColor(color.r, color.g, color.b, color.a));
		}

		Decimal getuniform_decimal(const std::string &name, const Decimal def) override final {
			auto it = uniformsFloat.find(name);
			if(it != uniformsFloat.end()) {
				return it->second;
			} else {
				setuniform(name, def);
				return def;
			}
		}

		Point3 getuniform_point3(const std::string &name, const Point3 def) override final {
			auto it = uniformsPoint3.find(name);
			if(it != uniformsPoint3.end()) {
				return it->second;
			} else {
				setuniform(name, def);
				return def;
			}
		}

		void setuniform(const std::string &name, glm::uint32 x, bool force = false) override final {
			auto it = uniformsU32.find(name);
			if(!force && it != uniformsU32.cend() && it->second == x) { return; }

			uniformsU32[name] = x;
			changedUniformsU32.emplace_back(name);
		}

		void setuniform(const std::string &name, Decimal x, bool force = false) override final {
			auto it = uniformsFloat.find(name);
			if(!force && it != uniformsFloat.cend() && it->second == x) { return; }

			uniformsFloat[name] = x;
			changedUniformsFloat.emplace_back(name);
		}

		void setuniform(const std::string &name, Point3 p, bool force = false) override final {
			auto it = uniformsPoint3.find(name);
			if(!force && it != uniformsPoint3.cend() && it->second == p) { return; }

			uniformsPoint3[name] = p;
			changedUniformsPoint3.emplace_back(name);
		}

		bool uploaduniform(GLuint program, const std::string &name, glm::int32 x) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			GL(glUniform1i(loc, x));
			return true;
		}

		bool uploaduniform(GLuint program, const std::string &name, glm::uint32 x) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			GL(glUniform1ui(loc, x));
			debugmanager()->trace("uniform ", name, " = ", x);
			return true;
		}

		bool uploaduniform(GLuint program, const std::string &name, Decimal x) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto x2 = float(x);
			GL(glUniform1f(loc, x2));
			debugmanager()->trace("uniform ", name, " = ", x);
			return true;
		}

		bool uploaduniform(GLuint program, const std::string &name, Point2 p) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto p2 = glm::vec2(p);
			GL(glUniform2f(loc, p2.x, p2.y));
			return true;
		}

		bool uploaduniform(GLuint program, const std::string &name, Point3 p) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto p2 = glm::vec3(p);
			GL(glUniform3f(loc, p2.x, p2.y, p2.z));
			return true;
		}

		bool uploaduniform(GLuint program, const std::string &name, Point4 p) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto p2 = glm::vec4(p);
			GL(glUniform4f(loc, p2.x, p2.y, p2.z, p2.w));
			return true;
		}

		bool uploaduniform(GLuint program, const std::string &name, Mat4 m) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto m2 = glm::mat4(m);
			GL(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m2)));
			return true;
		}
	};
} } }
