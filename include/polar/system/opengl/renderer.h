#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <polar/component/camera.h>
#include <polar/component/material.h>
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
		using models_type    = boost::container::flat_set<core::weak_ref>;
		using materials_type = boost::container::flat_map<core::weak_ref, models_type>;
		using stages_type    = boost::container::flat_map<core::weak_ref, materials_type>;
		using scenes_type    = std::map<core::weak_ref, stages_type>;

		// { scene => { stage => { material => { model } } } }
		scenes_type scenes;

		using cameras_type   = boost::container::flat_map<core::weak_ref, core::weak_ref>;
		using targets_type   = boost::container::flat_map<core::weak_ref, cameras_type>;

		// { target => { camera => scene } }
		targets_type targets;

		void update(DeltaTicks &) override {
			auto clock_ref = engine->own<tag::clock::simulation>();
			auto clock     = engine->add_as<component::clock::base, component::clock::simulation>(clock_ref);
			float delta    = clock->delta();

			for(auto &[target_ref, cameras] : targets) {
				if(auto fb = engine->get<component::opengl::framebuffer>(target_ref)) {
					GL(glBindFramebuffer(GL_FRAMEBUFFER, fb->fb));
				} else if(auto win = engine->get<component::opengl::window>(target_ref)) {
					SDL(SDL_GL_MakeCurrent(win->win, win->ctx));
					GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				}

				GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

				for(auto &[camera_ref, scene_ref] : cameras) {
					auto camera = engine->get<component::camera>(camera_ref);

					for(auto &[stage_ref, materials] : scenes[scene_ref]) {
						auto stage = engine->get<component::opengl::stage>(stage_ref);

						GL(glUseProgram(stage->program));

						for(auto &[material_ref, models] : materials) {
							auto material = engine->get<component::material>(material_ref);
							if(auto &diffuse_ref = material->diffuse) {
								if(auto fb = engine->get<component::opengl::framebuffer>(*diffuse_ref)) {
									GL(glActiveTexture(GL_TEXTURE0));
									GL(glBindTexture(GL_TEXTURE_2D, fb->tex));
									upload(stage->program, "u_diffuse_map", glm::int32(0));
								} else if(auto texture = engine->get<component::opengl::texture>(*diffuse_ref)) {
									GL(glActiveTexture(GL_TEXTURE0));
									GL(glBindTexture(GL_TEXTURE_2D, texture->tex));
									upload(stage->program, "u_diffuse_map", glm::int32(0));
								}
							}

							for(auto &model_ref : models) {
								auto model = engine->get<component::opengl::model>(model_ref);

								math::mat4x4 u_model(1);

								if(auto pos = engine->get<component::position>(model_ref)) {
									u_model = glm::translate(u_model, pos->pos.temporal(delta));
								}
								if(auto orient = engine->get<component::orientation>(model_ref)) {
									u_model *= glm::toMat4(glm::inverse(orient->orient.temporal(delta)));
								}
								if(auto sc = engine->get<component::scale>(model_ref)) {
									u_model = glm::scale(u_model, sc->sc.temporal(delta));
								}

								upload(stage->program, "u_model", u_model);

								GL(glBindVertexArray(model->vao));
								GL(glDrawArrays(GL_TRIANGLES, 0, model->count));
							}
						}
					}
				}

				if(auto win = engine->get<component::opengl::window>(target_ref)) {
					SDL(SDL_GL_SwapWindow(win->win));
				}
			}
		}

		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::window)) {
				targets.emplace(wr, cameras_type{});
			} else if(ti == typeid(component::opengl::framebuffer)) {
				targets.emplace(wr, cameras_type{});
			} else if(ti == typeid(component::camera)) {
				auto camera = std::static_pointer_cast<component::camera>(ptr.lock());
				targets[camera->target].emplace(wr, camera->scene);
			} else if(ti == typeid(component::opengl::model)) {
				if(auto model = engine->get<component::model>(wr)) {
					if(auto material = engine->get<component::material>(model->material)) {
						scenes[model->scene][material->stage][model->material].emplace(wr);
					}
				}
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::window)) {
				targets.erase(wr);
			} else if(ti == typeid(component::opengl::framebuffer)) {
				targets.erase(wr);
			} else if(ti == typeid(component::camera)) {
				auto camera = std::static_pointer_cast<component::camera>(ptr.lock());

				auto search = targets.find(camera->target);
				if(search != targets.end()) {
					search->second.erase(wr);
				}
			} else if(ti == typeid(component::opengl::model)) {
				if(auto model = engine->get<component::model>(wr)) {
					if(auto material = engine->get<component::material>(model->material)) {
						auto search = scenes.find(model->scene);
						if(search != scenes.end()) {
							auto search2 = search->second.find(material->stage);
							if(search2 != search->second.end()) {
								auto search3 = search2->second.find(model->material);
								if(search3 != search2->second.end()) {
									search3->second.erase(wr);
								}
							}
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
