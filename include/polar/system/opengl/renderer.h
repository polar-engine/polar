#pragma once

#include <polar/component/opengl/framebuffer.h>
#include <polar/component/opengl/stage.h>
#include <polar/component/opengl/texture.h>
#include <polar/component/opengl/window.h>
#include <polar/component/orientation.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/component/stage.h>

namespace polar::system::opengl {
	class renderer : public base {
	  private:
		using models_type = std::set<core::weak_ref>;
		using stages_type = std::map<core::weak_ref, models_type>;

		std::map<core::weak_ref, stages_type> targets;

		void update(DeltaTicks &) override {
			auto clock_ref = engine->own<tag::clock::simulation>();
			auto clock     = engine->add_as<component::clock::base, component::clock::simulation>(clock_ref);
			float delta    = clock->delta();

			for(auto &[target_ref, stages] : targets) {
				if(auto fb = engine->get<component::opengl::framebuffer>(target_ref)) {
					GL(glBindFramebuffer(GL_FRAMEBUFFER, fb->fb));
				} else if(auto win = engine->get<component::opengl::window>(target_ref)) {
					SDL(SDL_GL_MakeCurrent(win->win, win->ctx));
					GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				}

				GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

				for(auto &[stage_ref, models] : stages) {
					auto stage = engine->get<component::opengl::stage>(stage_ref);

					GL(glUseProgram(stage->program));

					for(auto &model_ref : models) {
						auto model = engine->get<component::opengl::model>(model_ref);

						// model matrix

						std::shared_ptr<component::position> pos;
						std::shared_ptr<component::orientation> orient;
						std::shared_ptr<component::scale> sc;

						auto ref_range = engine->objects.get<core::index::ref>().equal_range(model_ref);
						for(auto ref_it = ref_range.first; ref_it != ref_range.second; ++ref_it) {
							auto ti = ref_it->ti;
							if(ti == typeid(component::position)) {
								pos = std::static_pointer_cast<component::position>(ref_it->ptr);
							} else if(ti == typeid(component::orientation)) {
								orient = std::static_pointer_cast<component::orientation>(ref_it->ptr);
							} else if(ti == typeid(component::scale)) {
								sc = std::static_pointer_cast<component::scale>(ref_it->ptr);
							}
						}

						math::mat4x4 u_model(1);

						if(pos != nullptr) {
							u_model = glm::translate(u_model, pos->pos.temporal(delta));
						}
						if(orient != nullptr) {
							u_model *= glm::toMat4(glm::inverse(orient->orient.temporal(delta)));
						}
						if(sc != nullptr) {
							u_model = glm::scale(u_model, sc->sc.temporal(delta));
						}

						upload(stage->program, "u_model", u_model);

						// material
						if(auto material = engine->get<component::material>(model_ref)) {
							if(auto fb_diffuse = engine->get<component::opengl::framebuffer>(material->diffuse)) {
								GL(glActiveTexture(GL_TEXTURE0));
								GL(glBindTexture(GL_TEXTURE_2D, fb_diffuse->tex));
								upload(stage->program, "u_diffuse", glm::int32(0));
							} else if(auto texture = engine->get<component::opengl::texture>(material->diffuse)) {
								GL(glActiveTexture(GL_TEXTURE0));
								GL(glBindTexture(GL_TEXTURE_2D, texture->tex));
								upload(stage->program, "u_diffuse", glm::int32(0));
							}
						}

						GL(glBindVertexArray(model->vao));
						GL(glDrawArrays(GL_TRIANGLES, 0, model->count));
					}
				}

				if(auto win = engine->get<component::opengl::window>(target_ref)) {
					SDL(SDL_GL_SwapWindow(win->win));
				}
			}
		}

		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::window)) {
				targets.emplace(wr, stages_type{});
			} else if(ti == typeid(component::opengl::framebuffer)) {
				targets.emplace(wr, stages_type{});
			} else if(ti == typeid(component::opengl::model)) {
				if(auto model = engine->get<component::model>(wr)) {
					auto stage = engine->get<component::stage>(model->stage);
					targets[stage->target][model->stage].emplace(wr);
				}
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::window)) {
				targets.erase(wr);
			} else if(ti == typeid(component::opengl::framebuffer)) {
				targets.erase(wr);
			} else if(ti == typeid(component::stage)) {
				auto stage = std::static_pointer_cast<component::stage>(ptr.lock());

				auto search = targets.find(stage->target);
				if(search != targets.end()) {
					search->second.erase(wr);
				}
			} else if(ti == typeid(component::opengl::model)) {
				if(auto model = engine->get<component::model>(wr)) {
					auto stage = engine->get<component::stage>(model->stage);

					auto search = targets.find(stage->target);
					if(search != targets.end()) {
						auto search2 = search->second.find(model->stage);
						if(search2 != search->second.end()) {
							search2->second.erase(wr);
						}
					}
				}
			}
		}

		bool upload(GLuint program, const std::string &name, glm::int32 x) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			GL(glUniform1i(loc, x));
			return true;
		}

		bool upload(GLuint program, const std::string &name, glm::uint32 x) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			GL(glUniform1ui(loc, x));
			log()->trace("opengl::renderer", "uniform ", name, " = ", x);
			return true;
		}

		bool upload(GLuint program, const std::string &name, math::decimal x) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto x2 = float(x);
			GL(glUniform1f(loc, x2));
			log()->trace("opengl::renderer", "uniform ", name, " = ", x);
			return true;
		}

		bool upload(GLuint program, const std::string &name, math::point2 p) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto p2 = glm::vec2(p);
			GL(glUniform2f(loc, p2.x, p2.y));
			return true;
		}

		bool upload(GLuint program, const std::string &name, math::point3 p) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto p2 = glm::vec3(p);
			GL(glUniform3f(loc, p2.x, p2.y, p2.z));
			return true;
		}

		bool upload(GLuint program, const std::string &name, math::point4 p) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto p2 = glm::vec4(p);
			GL(glUniform4f(loc, p2.x, p2.y, p2.z, p2.w));
			return true;
		}

		bool upload(GLuint program, const std::string &name, math::mat4x4 m) {
			GLint loc;
			GL(loc = glGetUniformLocation(program, name.c_str()));
			if(loc == -1) { return false; } // -1 if uniform does not exist in program
			auto m2 = glm::mat4(m);
			GL(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m2)));
			return true;
		}

	  public:
		static bool supported() { return true; }

		renderer(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "opengl_renderer"; }
	};
} // namespace polar::system::opengl
