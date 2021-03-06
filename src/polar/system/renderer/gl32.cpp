#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <polar/asset/image.h>
#include <polar/asset/material.h>
#include <polar/asset/shaderprogram.h>
#include <polar/component/color.h>
#include <polar/component/orientation.h>
#include <polar/component/playercamera.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/text.h>
#include <polar/core/polar.h>
#include <polar/math/constants.h>
#include <polar/math/mat.h>
#include <polar/math/point.h>
#include <polar/support/action/controller.h>
#include <polar/support/action/keyboard.h>
#include <polar/support/action/mouse.h>
#include <polar/support/phys/detector/ball.h>
#include <polar/support/phys/detector/box.h>
#include <polar/system/asset.h>
#include <polar/system/integrator.h>
#include <polar/system/renderer/gl32.h>
#include <polar/system/vr.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

namespace polar::system::renderer {
	void GLAPIENTRY debugCB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	                        const GLchar *message, const void *userParam) {
		(void)source;
		(void)type;
		(void)id;
		(void)severity;
		(void)length;
		(void)userParam;
		log()->debug("gl", "OPENGL DEBUG OUTPUT: ", message);
	}

	bool gl32::supported() {
		gl32 renderer(nullptr, {});
		try {
			renderer.initGL();
			GLint major, minor;
			if(!GL(glGetIntegerv(GL_MAJOR_VERSION, &major))) {
				log()->fatal("gl", "failed to get OpenGL major version");
			}
			if(!GL(glGetIntegerv(GL_MINOR_VERSION, &minor))) {
				log()->fatal("gl", "failed to get OpenGL minor version");
			}
			/* if OpenGL version is 3.1 or greater */
			if(!(major > 3 || (major == 3 && minor >= 1))) {
				std::stringstream msg;
				msg << "actual OpenGL version is " << major << '.' << minor;
				log()->fatal("gl", msg.str());
			}
			return true;
		} catch(std::exception &) { return false; }
	}

	void gl32::initGL() {
		if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) { log()->fatal("gl", "failed to init SDL"); }
		if(!SDL(TTF_Init())) { log()->fatal("gl", "failed to init TTF"); }
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) {
			log()->fatal("gl", "failed to set major version attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1))) {
			log()->fatal("gl", "failed to set minor version attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) {
			log()->fatal("gl", "failed to set profile mask attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) {
			log()->fatal("gl", "failed to set double buffer attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG))) {
			log()->fatal("gl", "failed to set context flags");
		}
		Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
		                      SDL_WINDOW_RESIZABLE
		                      //                    | SDL_WINDOW_ALLOW_HIGHDPI
		                      | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
		if(!SDL(window = SDL_CreateWindow("Polar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
		                                  window_flags))) {
			log()->fatal("gl", "failed to create window");
		}
		if(!SDL(context = SDL_GL_CreateContext(window))) { log()->fatal("gl", "failed to create OpenGL context"); }
		if(!SDL(SDL_GL_SetSwapInterval(1))) { log()->critical("gl", "failed to set swap interval"); }

		if(!SDL(SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE))) {
			log()->fatal("gl", "failed to set relative mouse mode");
		}

		/* set up controller joysticks */
		SDL_GameController *controller = nullptr;
		for(int i = 0; i < SDL_NumJoysticks(); ++i) {
			bool isGameCon;
			SDL(isGameCon = SDL_IsGameController(i));
			if(isGameCon) {
				log()->verbose("gl", "SDL detected controller #", i);
				SDL(controller = SDL_GameControllerOpen(i));
			}
		}
		SDL(SDL_GameControllerEventState(SDL_ENABLE));

		glewExperimental = GL_TRUE;
		GLenum err       = glewInit();

		if(err != GLEW_OK) { log()->fatal("gl", "GLEW: glewInit failed"); }

		/* GLEW cals glGetString(EXTENSIONS) which
		 * causes GL_INVALID_ENUM on GL 3.2+ core contexts
		 */
		glGetError();

		GL(glEnable(GL_DEPTH_TEST));
		GL(glEnable(GL_BLEND));
		GL(glEnable(GL_CULL_FACE));
		GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GL(glCullFace(GL_BACK));

		if(glewIsExtensionSupported("KHR_debug")) {
			GL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
			GL(glDebugMessageCallback(debugCB, nullptr));
			GL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE));
		}
	}

	void gl32::init() {
		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			width  = vr->width();
			height = vr->height();
		}
		initGL();

		setclearcolor(math::point4(0.0f));

		GL(glGenVertexArrays(1, &viewportVAO));
		GL(glBindVertexArray(viewportVAO));

		GLuint viewport_vbo;
		GL(glGenBuffers(1, &viewport_vbo));

		viewportPoints.clear();
		const float step = 0.1f;
		for(float x = -1; x < 1; x += step) {
			for(float y = -1; y < 1; y += step) {
				viewportPoints.emplace_back(math::point2(x, y));
				viewportPoints.emplace_back(math::point2(x + step, y));
				viewportPoints.emplace_back(math::point2(x, y + step));
				viewportPoints.emplace_back(math::point2(x, y + step));
				viewportPoints.emplace_back(math::point2(x + step, y));
				viewportPoints.emplace_back(math::point2(x + step, y + step));
			}
		}

		GL(glBindBuffer(GL_ARRAY_BUFFER, viewport_vbo));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * viewportPoints.size(), viewportPoints.data(),
		                GL_STATIC_DRAW));
		GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL));
		GL(glEnableVertexAttribArray(0));

		// debug box

		GL(glGenVertexArrays(1, &debug_box_vao));
		GL(glBindVertexArray(debug_box_vao));

		GLuint debug_box_vbo;
		GL(glGenBuffers(1, &debug_box_vbo));

		debug_box_points.clear();

		// front
		debug_box_points.emplace_back(math::point3(-1, -1, 1));
		debug_box_points.emplace_back(math::point3(1, -1, 1));
		debug_box_points.emplace_back(math::point3(-1, 1, 1));
		debug_box_points.emplace_back(math::point3(-1, 1, 1));
		debug_box_points.emplace_back(math::point3(1, -1, 1));
		debug_box_points.emplace_back(math::point3(1, 1, 1));

		// left
		debug_box_points.emplace_back(math::point3(-1, -1, -1));
		debug_box_points.emplace_back(math::point3(-1, -1, 1));
		debug_box_points.emplace_back(math::point3(-1, 1, -1));
		debug_box_points.emplace_back(math::point3(-1, 1, -1));
		debug_box_points.emplace_back(math::point3(-1, -1, 1));
		debug_box_points.emplace_back(math::point3(-1, 1, 1));

		// back
		debug_box_points.emplace_back(math::point3(1, -1, -1));
		debug_box_points.emplace_back(math::point3(-1, -1, -1));
		debug_box_points.emplace_back(math::point3(1, 1, -1));
		debug_box_points.emplace_back(math::point3(1, 1, -1));
		debug_box_points.emplace_back(math::point3(-1, -1, -1));
		debug_box_points.emplace_back(math::point3(-1, 1, -1));

		// right
		debug_box_points.emplace_back(math::point3(1, -1, 1));
		debug_box_points.emplace_back(math::point3(1, -1, -1));
		debug_box_points.emplace_back(math::point3(1, 1, 1));
		debug_box_points.emplace_back(math::point3(1, 1, 1));
		debug_box_points.emplace_back(math::point3(1, -1, -1));
		debug_box_points.emplace_back(math::point3(1, 1, -1));

		// top
		debug_box_points.emplace_back(math::point3(-1, 1, 1));
		debug_box_points.emplace_back(math::point3(1, 1, 1));
		debug_box_points.emplace_back(math::point3(-1, 1, -1));
		debug_box_points.emplace_back(math::point3(-1, 1, -1));
		debug_box_points.emplace_back(math::point3(1, 1, 1));
		debug_box_points.emplace_back(math::point3(1, 1, -1));

		// bottom
		debug_box_points.emplace_back(math::point3(-1, -1, -1));
		debug_box_points.emplace_back(math::point3(1, -1, -1));
		debug_box_points.emplace_back(math::point3(-1, -1, 1));
		debug_box_points.emplace_back(math::point3(-1, -1, 1));
		debug_box_points.emplace_back(math::point3(1, -1, -1));
		debug_box_points.emplace_back(math::point3(1, -1, 1));

		GL(glBindBuffer(GL_ARRAY_BUFFER, debug_box_vbo));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * debug_box_points.size(), debug_box_points.data(),
		                GL_STATIC_DRAW));
		GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));
		GL(glEnableVertexAttribArray(0));

		// debug ball

		GL(glGenVertexArrays(1, &debug_ball_vao));
		GL(glBindVertexArray(debug_ball_vao));

		GLuint debug_ball_vbo;
		GL(glGenBuffers(1, &debug_ball_vbo));

		debug_ball_points.clear();

		math::point3 up(0, 1, 0);
		math::point3 right(1, 0, 0);
		math::point3 dir(0, 0, -1);

		constexpr math::decimal k = math::PI_OVER<10>;

		for(math::decimal phi = -math::PI_OVER<2>; phi < math::PI_OVER<2>; phi += k) {
			for(math::decimal theta = 0; theta < math::TWO_PI; theta += k) {
				auto p = glm::rotate(glm::rotate(dir, phi, right), theta, up);
				auto q = glm::rotate(glm::rotate(dir, phi, right), theta + k, up);
				auto r = glm::rotate(glm::rotate(dir, phi + k, right), theta, up);
				auto s = glm::rotate(glm::rotate(dir, phi + k, right), theta + k, up);

				debug_ball_points.emplace_back(p);
				debug_ball_points.emplace_back(q);
				debug_ball_points.emplace_back(r);
				debug_ball_points.emplace_back(r);
				debug_ball_points.emplace_back(q);
				debug_ball_points.emplace_back(s);
			}
		}

		GL(glBindBuffer(GL_ARRAY_BUFFER, debug_ball_vbo));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * debug_ball_points.size(), debug_ball_points.data(),
		                GL_STATIC_DRAW));
		GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));
		GL(glEnableVertexAttribArray(0));

		log()->trace("gl", "MakePipeline from Init");
		makepipeline(pipelineNames);
		log()->trace("gl", "MakePipeline done");

		auto assetM     = engine->get<asset>().lock();
		spriteProgram   = makeprogram(assetM->get<polar::asset::shaderprogram>("sprite"));
		identityProgram = makeprogram(assetM->get<polar::asset::shaderprogram>("identity"));
		debugProgram    = makeprogram(assetM->get<polar::asset::shaderprogram>("debug"));

		/* 8x8 Bayer ordered dithering pattern
		 * each input pixel is scaled to the range of 0->63 before lookup
		 */
		static const char ditherPattern[] = {0,  32, 8,  40, 2,  34, 10, 42, 48, 16, 56, 24, 50, 18, 58, 26,
		                                     12, 44, 4,  36, 14, 46, 6,  38, 60, 28, 52, 20, 62, 30, 54, 22,
		                                     3,  35, 11, 43, 1,  33, 9,  41, 51, 19, 59, 27, 49, 17, 57, 25,
		                                     15, 47, 7,  39, 13, 45, 5,  37, 63, 31, 55, 23, 61, 29, 53, 21};

		GL(glGenTextures(1, &ditherTex));
		GL(glBindTexture(GL_TEXTURE_2D, ditherTex));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 8, 8, 0, GL_RED, GL_UNSIGNED_BYTE, ditherPattern));

		inited = true;
	}

	void gl32::render(math::mat4x4 proj, math::mat4x4 view, float delta) {
		auto assetM = engine->get<asset>().lock();

		std::unordered_map<std::string, GLuint> globals;
		for(unsigned int i = 0; i < nodes.size(); ++i) {
			auto &node = nodes[i];
			project(node.program, proj);

			GL(glBindFramebuffer(GL_FRAMEBUFFER, node.fbo));
			GL(glUseProgram(node.program));

			GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			uploaduniform(node.program, "u_view", view);
			uploaduniform(node.program, "u_invViewProj", glm::inverse(proj * view));

			unsigned int texPos = 0;
			GL(glActiveTexture(GL_TEXTURE0 + texPos));
			GL(glBindTexture(GL_TEXTURE_2D, ditherTex));

			uploaduniform(node.program, "u_ditherTex", glm::int32(texPos));
			++texPos;

			auto diffuse_pos = texPos;
			++texPos;

			auto specular_pos = texPos;
			++texPos;

			auto normal_pos = texPos;
			++texPos;

			switch(i) {
			case 0: {
				auto ti_range = engine->objects.get<core::index::ti>().equal_range(typeid(component::model));
				for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
					auto model                     = static_cast<component::model *>(ti_it->ptr.get());
					component::position *pos       = nullptr;
					component::orientation *orient = nullptr;
					component::scale *sc           = nullptr;

					auto ref_range = engine->objects.get<core::index::ref>().equal_range(ti_it->r);
					for(auto ref_it = ref_range.first; ref_it != ref_range.second; ++ref_it) {
						auto type = ref_it->ti;
						if(type == typeid(component::position)) {
							pos = static_cast<component::position *>(ref_it->ptr.get());
						} else if(type == typeid(component::orientation)) {
							orient = static_cast<component::orientation *>(ref_it->ptr.get());
						} else if(type == typeid(component::scale)) {
							sc = static_cast<component::scale *>(ref_it->ptr.get());
						}
					}

					auto property = model->get<model_p>();
					if(property) {
						math::mat4x4 modelMatrix(1);

						if(pos != nullptr) { modelMatrix = glm::translate(modelMatrix, pos->pos.temporal(delta)); }
						if(orient != nullptr) {
							modelMatrix *= glm::toMat4(glm::inverse(orient->orient.temporal(delta)));
						}
						if(sc != nullptr) { modelMatrix = glm::scale(modelMatrix, sc->sc.temporal(delta)); }

						GLenum drawMode = GL_TRIANGLES;

						uploaduniform(node.program, "u_model", modelMatrix);

						if(model->asset->material) {
							auto mat = assetM->get<polar::asset::material>(*model->asset->material);
							uploaduniform(node.program, "u_ambient", mat->ambient);
							uploaduniform(node.program, "u_diffuse", mat->diffuse);
							uploaduniform(node.program, "u_specular", mat->specular);
							uploaduniform(node.program, "u_specular_exponent", mat->specular_exponent);
						}

						GL(glActiveTexture(GL_TEXTURE0 + diffuse_pos));
						GL(glBindTexture(GL_TEXTURE_2D, property->diffuse_map));
						uploaduniform(node.program, "u_diffuse_map", glm::int32(diffuse_pos));

						GL(glActiveTexture(GL_TEXTURE0 + specular_pos));
						GL(glBindTexture(GL_TEXTURE_2D, property->specular_map));
						uploaduniform(node.program, "u_specular_map", glm::int32(specular_pos));

						GL(glActiveTexture(GL_TEXTURE0 + normal_pos));
						GL(glBindTexture(GL_TEXTURE_2D, property->normal_map));
						uploaduniform(node.program, "u_normal_map", glm::int32(normal_pos));

						GL(glBindVertexArray(property->vao));
						GL(glDrawArrays(drawMode, 0, property->numVertices));
					}
				}
				if(debug_draw) {
					GL(glUseProgram(debugProgram));

					project(debugProgram, proj);
					uploaduniform(debugProgram, "u_view", view);

					for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
						auto objectID = ti_it->r;
						auto phys     = engine->get<component::phys>(objectID);
						auto pos      = engine->get<component::position>(objectID);
						auto sc       = engine->get<component::scale>(objectID);

						if(phys != nullptr) {
							math::mat4x4 modelMatrix(1);

							if(pos != nullptr) { modelMatrix = glm::translate(modelMatrix, pos->pos.temporal(delta)); }
							if(sc != nullptr) { modelMatrix = glm::scale(modelMatrix, sc->sc.temporal(delta)); }
							if(phys->detector) { modelMatrix = glm::scale(modelMatrix, phys->detector->size); }

							uploaduniform(debugProgram, "u_model", modelMatrix);

							auto &det = *phys->detector;
							auto ti   = std::type_index(typeid(det));
							if(ti == typeid(support::phys::detector::box)) {
								GL(glBindVertexArray(debug_box_vao));
								GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(debug_box_points.size())));
							} else if(ti == typeid(support::phys::detector::ball)) {
								GL(glBindVertexArray(debug_ball_vao));
								GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(debug_ball_points.size())));
							}
						}
					}
				}
				break;
			}
			default:
				// for each previous global output
				for(auto &pair : nodes[i - 1].globalOuts) { globals.emplace(pair); }

				// for each input
				for(auto &pair : node.ins) {
					auto buffer = nodes[i - 1].outs[pair.first];
					GL(glActiveTexture(GL_TEXTURE0 + texPos));
					GL(glBindTexture(GL_TEXTURE_2D, buffer));

					uploaduniform(node.program, pair.second, glm::int32(texPos));
					++texPos;
				}

				// for each globla input
				for(auto &pair : node.globalIns) {
					auto buffer = globals[pair.first];
					GL(glActiveTexture(GL_TEXTURE0 + texPos));
					GL(glBindTexture(GL_TEXTURE_2D, buffer));

					uploaduniform(node.program, pair.second, glm::int32(texPos));
					++texPos;
				}

				GL(glBindVertexArray(viewportVAO));
				GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(viewportPoints.size())));

				break;
			}
		}

		// render sprites and text
		// GL(glEnable(GL_BLEND));
		{
			// GL(glBindFramebuffer(GL_FRAMEBUFFER, nodes.back().fbo));
			GL(glUseProgram(spriteProgram));
			uploaduniform(spriteProgram, "u_texture", 0);
			GL(glActiveTexture(GL_TEXTURE0));
			GL(glBindVertexArray(viewportVAO));

			auto ti_range = engine->objects.get<core::index::ti>().equal_range(typeid(component::sprite::base));
			for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
				rendersprite(ti_it->r, proj, view);
			}

			ti_range = engine->objects.get<core::index::ti>().equal_range(typeid(component::text));
			for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
				rendertext(ti_it->r, proj, view);
			}
		}
		// GL(glDisable(GL_BLEND));

		// mirror
		GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL(glUseProgram(identityProgram));
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL(glActiveTexture(GL_TEXTURE0));
		GL(glBindTexture(GL_TEXTURE_2D, nodes.back().outs.at("color")));
		uploaduniform(identityProgram, "u_colorBuffer", 0);
		GL(glBindVertexArray(viewportVAO));
		GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(viewportPoints.size())));
	}

	void gl32::update(DeltaTicks &dt) {
		// upload changed uniforms
		for(size_t i = 0; i < nodes.size(); ++i) {
			bool usingProgram = false;
			auto &node        = nodes[nodes.size() - 1 - i];
			for(auto &name : changedUniformsU32) {
				if(node.uniforms.find(name) != node.uniforms.cend()) {
					if(!usingProgram) {
						GL(glUseProgram(node.program));
						usingProgram = true;
					}
					uploaduniform(node.program, name, uniformsU32[name]);
				}
			}
			for(auto &name : changedUniformsFloat) {
				if(node.uniforms.find(name) != node.uniforms.cend()) {
					if(!usingProgram) {
						GL(glUseProgram(node.program));
						usingProgram = true;
					}
					uploaduniform(node.program, name, uniformsFloat[name]);
				}
			}
			for(auto &name : changedUniformsPoint3) {
				if(node.uniforms.find(name) != node.uniforms.cend()) {
					if(!usingProgram) {
						GL(glUseProgram(node.program));
						usingProgram = true;
					}
					uploaduniform(node.program, name, uniformsPoint3[name]);
				}
			}
		}
		changedUniformsU32.clear();
		changedUniformsFloat.clear();
		changedUniformsPoint3.clear();

		fps_object = engine->add();

		if(dt.Seconds() > 0) { fps = glm::mix(fps, 1 / dt.Seconds(), math::decimal(0.1)); }

		if(showFPS) {
			std::ostringstream oss;
			oss << (int)fps << " fps";

			auto assetM = engine->get<asset>().lock();
			auto font   = assetM->get<polar::asset::font>("nasalization-rg");

			engine->add<component::text>(fps_object, font, oss.str());
			engine->add<component::screenposition>(fps_object, math::point2(5, 5), support::ui::origin::topleft);
			engine->add<component::color>(fps_object, math::point4(1, 1, 1, 0.8));
			engine->add<component::scale>(fps_object, math::point3(0.125));
		}

		auto clock_ref = engine->own<tag::clock::simulation>();
		auto clock     = engine->add_as<component::clock::base, component::clock::simulation>(clock_ref);
		float delta    = clock->delta();

		math::mat4x4 cameraView(1);

		auto ti_range = engine->objects.get<core::index::ti>().equal_range(typeid(component::playercamera));
		for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
			auto camera = static_cast<component::playercamera *>(ti_it->ptr.get());

			component::position *pos       = nullptr;
			component::orientation *orient = nullptr;
			// component::scale *sc           = nullptr;

			auto ref_range = engine->objects.get<core::index::ref>().equal_range(ti_it->r);
			for(auto ref_it = ref_range.first; ref_it != ref_range.second; ++ref_it) {
				auto type = ref_it->ti;
				if(type == typeid(component::position)) {
					pos = static_cast<component::position *>(ref_it->ptr.get());
				} else if(type == typeid(component::orientation)) {
					orient = static_cast<component::orientation *>(ref_it->ptr.get());
				}
			}

			cameraView = glm::translate(cameraView, -camera->distance.temporal(delta));
			cameraView *= glm::toMat4(camera->orientation.temporal(delta));
			if(orient != nullptr) { cameraView *= glm::toMat4(orient->orient.temporal(delta)); }
			cameraView = glm::translate(cameraView, -camera->position.temporal(delta));
			if(pos != nullptr) { cameraView = glm::translate(cameraView, -pos->pos.temporal(delta)); }
		}

		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			using eye = support::vr::eye;

			vr->update_poses();

			cameraView = glm::transpose(vr->head_view()) * cameraView;

			render(vr->projection(eye::left, zNear, zFar), cameraView, delta);
			GL(vr->submit_gl(eye::left, nodes.back().outs.at("color")));
			render(vr->projection(eye::right, zNear, zFar), cameraView, delta);
			GL(vr->submit_gl(eye::right, nodes.back().outs.at("color")));
		} else {
			render(calculate_projection(), cameraView, delta);
		}

		SDL(SDL_GL_SwapWindow(window));

		// handle input at beginning of frame to reduce delays in other systems
		SDL_Event event;
		while(SDL_PollEvent(&event)) { handleSDL(event); }
		SDL_ClearError();
	}

	void gl32::rendersprite(core::weak_ref object, math::mat4x4 proj, math::mat4x4 view) {
		using origin_t = support::ui::origin;

		auto sprite    = engine->get<component::sprite::base>(object);
		auto prop      = sprite->get<property::gl32::sprite>();
		auto screenPos = engine->get<component::screenposition>(object);
		auto scale     = engine->get<component::scale>(object);
		auto color     = engine->get<component::color>(object);

		auto coord = math::point2(0);

		// anchor point + local position component - alignment offset
		if(screenPos) {
			auto p = screenPos->position.get();
			switch(screenPos->origin) {
			case origin_t::bottomleft:
				coord = math::point2(0, 0);
				coord += p * math::point2(1, 1);
				break;
			case origin_t::bottomright:
				coord = math::point2(width, 0);
				coord += p * math::point2(-1, 1);
				break;
			case origin_t::topleft:
				coord = math::point2(0, height);
				coord += p * math::point2(1, -1);
				break;
			case origin_t::topright:
				coord = math::point2(width, height);
				coord += p * math::point2(-1, -1);
				break;
			case origin_t::left:
				coord = math::point2(0, height / 2);
				coord += p * math::point2(1, 0);
				break;
			case origin_t::right:
				coord = math::point2(width, height / 2);
				coord += p * math::point2(-1, 0);
				break;
			case origin_t::bottom:
				coord = math::point2(width / 2, 0);
				coord += p * math::point2(0, 1);
				break;
			case origin_t::top:
				coord = math::point2(width / 2, height);
				coord += p * math::point2(0, -1);
				break;
			case origin_t::center:
				coord = math::point2(width / 2, height / 2);
				coord += p * math::point2(1, 1);
				break;
			}
		}

		// offset y coord away from anchor point
		math::decimal offsetY = 0;
		if(screenPos) {
			switch(screenPos->origin) {
			default:
				break;
			case origin_t::topleft:
			case origin_t::top:
			case origin_t::topright:
				offsetY = -1;
				break;
			case origin_t::bottomleft:
			case origin_t::bottom:
			case origin_t::bottomright:
				offsetY = 1;
				break;
			}
		}

		math::mat4x4 transform(1);

		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			glm::vec3 mScale;
			glm::quat mRotation;
			glm::vec3 mTranslation;
			glm::vec3 mSkew;
			glm::vec4 mPerspective;
			glm::decompose(view, mScale, mRotation, mTranslation, mSkew, mPerspective);

			// static glm::quat rot{1, 0, 0, 0};
			// auto delta = glm::inverse(rot) * mRotation;
			// rot *= glm::pow(delta, math::decimal(0.002));

			// map to plane in 3D space
			transform = proj;
			if(!sprite->fixedToViewport) { transform *= glm::inverse(glm::toMat4(mRotation)); }
			// transform = glm::rotate(transform, theta, math::point3(0, 1, 0));
			transform = glm::translate(transform, math::point3(0, 0, -(zNear + zFar) / 2.0));
			transform = glm::scale(transform, math::point3(2.0 / width, 2.0 / height, 1));

			// scale down for VR viewing
			math::decimal uiScale = 1;
			uiScale /= 8;
			transform = glm::scale(transform, math::point3(uiScale, uiScale, 1));
			transform = glm::scale(transform, math::point3(1, 1, (zFar - zNear) / 4.0));
		}

		// translate to bottom left corner
		transform = glm::translate(transform, math::point3(-1, -1, 0));

		// normalize bounds
		transform = glm::scale(transform, math::point3(2, 2, 1));

		// scale to one pixel
		transform = glm::scale(transform, math::point3(1.0 / width, 1.0 / height, 1));

		// translate to screen coord
		transform = glm::translate(transform, math::point3(coord.x, coord.y, 0));

		// scale by scale component
		if(scale) { transform = glm::scale(transform, scale->sc.get()); }

		uploaduniform(spriteProgram, "u_color", color ? color->col.get() : math::point4(1));

		// scale to sprite size
		auto sc   = math::point3(sprite->surface->w, sprite->surface->h, 1);
		transform = glm::scale(transform, sc / math::decimal(2));

		// translate by one sprite size
		transform = glm::translate(transform, math::point3(1, offsetY, 0));

		uploaduniform(spriteProgram, "u_transform", transform);

		GL(glBindTexture(GL_TEXTURE_2D, prop->texture));
		GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(viewportPoints.size())));
	}

	void gl32::rendertext(core::weak_ref object, math::mat4x4 proj, math::mat4x4 view) {
		using origin_t = support::ui::origin;

		auto text      = engine->get<component::text>(object);
		auto screenPos = engine->get<component::screenposition>(object);
		auto scale     = engine->get<component::scale>(object);
		auto color     = engine->get<component::color>(object);

		if(!text || text->str.empty()) { return; }

		// calculate intended width of string
		math::decimal stringWidth = 0;
		for(auto c : text->str) { stringWidth += text->as->glyphs[c].advance; }

		// add required padding to string width
		if(screenPos) {
			auto &last = text->as->glyphs[text->str.back()];
			switch(screenPos->origin) {
			default:
				break;
			case origin_t::bottomright:
			case origin_t::right:
			case origin_t::topright:
				stringWidth += last.max.x / 2;
				break;
			case origin_t::bottom:
			case origin_t::top:
			case origin_t::center:
				stringWidth += last.max.x / 4;
				break;
			}
		}

		// scale string width by scale component
		if(scale) { stringWidth *= scale->sc.get().x; }

		auto coord = math::point2(0);

		// anchor point + local position component - alignment offset
		if(screenPos) {
			auto p = screenPos->position.get();
			switch(screenPos->origin) {
			case origin_t::bottomleft:
				coord = math::point2(0, 0);
				coord += p * math::point2(1, 1);
				coord.x -= 0;
				break;
			case origin_t::bottomright:
				coord = math::point2(width, 0);
				coord += p * math::point2(-1, 1);
				coord.x -= stringWidth;
				break;
			case origin_t::topleft:
				coord = math::point2(0, height);
				coord += p * math::point2(1, -1);
				coord.x -= 0;
				break;
			case origin_t::topright:
				coord = math::point2(width, height);
				coord += p * math::point2(-1, -1);
				coord.x -= stringWidth;
				break;
			case origin_t::left:
				coord = math::point2(0, height / 2);
				coord += p * math::point2(1, 0);
				coord.x -= 0;
				break;
			case origin_t::right:
				coord = math::point2(width, height / 2);
				coord += p * math::point2(-1, 0);
				coord.x -= stringWidth;
				break;
			case origin_t::bottom:
				coord = math::point2(width / 2, 0);
				coord += p * math::point2(0, 1);
				coord.x -= stringWidth / 2;
				break;
			case origin_t::top:
				coord = math::point2(width / 2, height);
				coord += p * math::point2(0, -1);
				coord.x -= stringWidth / 2;
				break;
			case origin_t::center:
				coord = math::point2(width / 2, height / 2);
				coord += p * math::point2(1, 1);
				coord.x -= stringWidth / 2;
				break;
			}
		}

		// offset y coord away from anchor point
		math::decimal offsetY = 0;
		if(screenPos) {
			switch(screenPos->origin) {
			default:
				break;
			case origin_t::topleft:
			case origin_t::top:
			case origin_t::topright:
				offsetY = -1;
				break;
			case origin_t::bottomleft:
			case origin_t::bottom:
			case origin_t::bottomright:
				offsetY = 1;
				break;
			}
		}

		math::mat4x4 transform(1);

		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			glm::vec3 mScale;
			glm::quat mRotation;
			glm::vec3 mTranslation;
			glm::vec3 mSkew;
			glm::vec4 mPerspective;
			glm::decompose(view, mScale, mRotation, mTranslation, mSkew, mPerspective);

			// static glm::quat rot{1, 0, 0, 0};
			// auto delta = glm::inverse(rot) * mRotation;
			// rot *= glm::pow(delta, math::decimal(0.002));

			// map to plane in 3D space
			transform = proj;
			if(!text->fixedToViewport) { transform *= glm::inverse(glm::toMat4(mRotation)); }
			// transform = glm::rotate(transform, theta, math::point3(0, 1, 0));
			transform = glm::translate(transform, math::point3(0, 0, -(zNear + zFar) / 2.0));
			transform = glm::scale(transform, math::point3(2.0 / width, 2.0 / height, 1));

			// scale down for VR viewing
			math::decimal uiScale = 1;
			uiScale /= 8;
			transform = glm::scale(transform, math::point3(uiScale, uiScale, 1));
			transform = glm::scale(transform, math::point3(1, 1, (zFar - zNear) / 4.0));
		}

		// translate to bottom left corner
		transform = glm::translate(transform, math::point3(-1, -1, 0));

		// normalize bounds
		transform = glm::scale(transform, math::point3(2, 2, 1));

		// scale to one pixel
		transform = glm::scale(transform, math::point3(1.0 / width, 1.0 / height, 1));

		// translate to screen coord
		transform = glm::translate(transform, math::point3(coord.x, coord.y, 0));

		// scale by scale component
		if(scale) { transform = glm::scale(transform, scale->sc.get()); }

		uploaduniform(spriteProgram, "u_color", color ? color->col.get() : math::point4(1));

		math::decimal pen = 0;
		for(auto c : text->str) {
			math::mat4x4 glyphTransform = transform;

			// translate by glyph pen
			glyphTransform = glm::translate(glyphTransform, math::point3(pen, 0, 0));

			// translate by glyph min x
			glyphTransform = glm::translate(glyphTransform, math::point3(text->as->glyphs[c].min.x, 0, 0));

			// scale to glyph size
			auto sc        = math::point3(text->as->glyphs[c].surface->w, text->as->glyphs[c].surface->h, 1);
			glyphTransform = glm::scale(glyphTransform, sc / math::decimal(2));

			// translate by one glyph size
			glyphTransform = glm::translate(glyphTransform, math::point3(1, offsetY, 0));

			uploaduniform(spriteProgram, "u_transform", glyphTransform);

			GL(glBindTexture(GL_TEXTURE_2D, fontCache[text->as].entries[c].texture));
			GL(glDrawArrays(GL_TRIANGLES, 0, GLsizei(viewportPoints.size())));

			pen += text->as->glyphs[c].advance;
		}
	}

	gl32::~gl32() {
		SDL(SDL_GL_DeleteContext(context));
		SDL(SDL_DestroyWindow(window));
		SDL(SDL_GL_ResetAttributes());
		SDL(SDL_Quit());
	}

	void gl32::handleSDL(SDL_Event &ev) {
		namespace kb         = support::action::keyboard;
		namespace mouse      = support::action::mouse;
		namespace controller = support::action::controller;

		support::input::key key;

		auto act = engine->get<action>().lock();
		auto vr  = engine->get<system::vr>().lock();

		switch(ev.type) {
		case SDL_QUIT:
			engine->quit();
			break;
		case SDL_WINDOWEVENT:
			switch(ev.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				width  = ev.window.data1;
				height = ev.window.data2;

				if(vr && vr->ready()) {
					width  = vr->width();
					height = vr->height();
				} else {
					int w, h;
					SDL(SDL_GL_GetDrawableSize(window, &w, &h));
					width  = w;
					height = h;
				}

				rebuild();

				break;
			}
			break;
		case SDL_KEYDOWN:
			if(ev.key.repeat == 0) {
				key = mkKeyFromSDL(ev.key.keysym.sym);
				if(act) { act->trigger_digital(kb::key_ti(key), true); }
			}
			break;
		case SDL_KEYUP:
			key = mkKeyFromSDL(ev.key.keysym.sym);
			if(act) { act->trigger_digital(kb::key_ti(key), false); }
			break;
		case SDL_MOUSEBUTTONDOWN:
			key = mkMouseButtonFromSDL(ev.button.button);
			if(act) { act->trigger_digital(kb::key_ti(key), true); }
			break;
		case SDL_MOUSEBUTTONUP:
			key = mkMouseButtonFromSDL(ev.button.button);
			if(act) { act->trigger_digital(kb::key_ti(key), false); }
			break;
		case SDL_MOUSEMOTION:
			if(act) {
				// XXX: this is hacky
				act->accumulate<mouse::position_x>(math::decimal(ev.motion.x));
				act->accumulate<mouse::position_y>(math::decimal(ev.motion.y));

				act->accumulate<mouse::motion_x>(math::decimal(ev.motion.xrel));
				act->accumulate<mouse::motion_y>(math::decimal(ev.motion.yrel));
			}
			break;
		case SDL_MOUSEWHEEL:
			if(act) {
				act->accumulate<mouse::wheel_x>(math::decimal(ev.wheel.x));
				act->accumulate<mouse::wheel_y>(math::decimal(ev.wheel.y));
			}
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			key = mkButtonFromSDL(static_cast<SDL_GameControllerButton>(ev.cbutton.button));
			if(act) { act->trigger_digital(kb::key_ti(key), true); }
			break;
		case SDL_CONTROLLERBUTTONUP:
			key = mkButtonFromSDL(static_cast<SDL_GameControllerButton>(ev.cbutton.button));
			if(act) { act->trigger_digital(kb::key_ti(key), false); }
			break;
		case SDL_CONTROLLERAXISMOTION:
			switch(ev.caxis.axis) {
			case 0: // x axis
				if(act) { act->accumulate<controller::motion_x>(ev.caxis.value); }
				break;
			case 1: // y axis
				if(act) { act->accumulate<controller::motion_y>(ev.caxis.value); }
				break;
			}
			break;
		}
	}

	void gl32::makepipeline(const std::vector<std::string> &names) {
		pipelineNames = names;

		auto assetM = engine->get<asset>().lock();
		std::vector<std::shared_ptr<polar::asset::shaderprogram>> assets;
		for(auto &node : nodes) {
			for(auto &out : node.globalOuts) { GL(glDeleteTextures(1, &out.second)); }
			for(auto &out : node.outs) { GL(glDeleteTextures(1, &out.second)); }
			GL(glDeleteFramebuffers(1, &node.fbo));
			GL(glDeleteProgram(node.program));
		}
		nodes.clear();

		for(auto &name : names) {
			log()->verbose("gl", "building shader program `", name, '`');
			auto as = assetM->get<polar::asset::shaderprogram>(name);
			assets.emplace_back(as);
			nodes.emplace_back(makeprogram(as));
			for(auto &uniform : as->uniforms) { nodes.back().uniforms.emplace(uniform); }
		}

		for(unsigned int i = 0; i < nodes.size(); ++i) {
			auto &as   = assets[i];
			auto &node = nodes[i];

			std::vector<GLenum> drawBuffers;
			int colorAttachment = 0;

			GL(glGenFramebuffers(1, &node.fbo));
			GL(glBindFramebuffer(GL_FRAMEBUFFER, node.fbo));

			auto fOut = [this, &drawBuffers, &colorAttachment](polar::asset::shaderoutput &out) {
				using outputtype = support::shader::outputtype;

				GLuint texture;
				GL(glGenTextures(1, &texture));
				GL(glBindTexture(GL_TEXTURE_2D, texture));

				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

				GLenum internalFormat = 0, format = 0, type = 0, attachment = 0;

				/* WARNING: 3-component sized internal formats are not
				 * required to be renderable */
				switch(out.type) {
				case outputtype::rgb8:
					internalFormat = GL_RGB8;
					format         = GL_RGB;
					type           = GL_UNSIGNED_BYTE;
					attachment     = GL_COLOR_ATTACHMENT0;
					break;
				case outputtype::rgba8:
					internalFormat = GL_RGBA8;
					format         = GL_RGBA;
					type           = GL_UNSIGNED_BYTE;
					attachment     = GL_COLOR_ATTACHMENT0;
					break;
				case outputtype::rgb16f:
					internalFormat = GL_RGB16F;
					format         = GL_RGB;
					type           = GL_HALF_FLOAT;
					attachment     = GL_COLOR_ATTACHMENT0;
					break;
				case outputtype::rgba16f:
					internalFormat = GL_RGBA16F;
					format         = GL_RGBA;
					type           = GL_HALF_FLOAT;
					attachment     = GL_COLOR_ATTACHMENT0;
					break;
				case outputtype::rgb32f:
					internalFormat = GL_RGB32F;
					format         = GL_RGB;
					type           = GL_FLOAT;
					attachment     = GL_COLOR_ATTACHMENT0;
					break;
				case outputtype::rgba32f:
					internalFormat = GL_RGBA32F;
					format         = GL_RGBA;
					type           = GL_FLOAT;
					attachment     = GL_COLOR_ATTACHMENT0;
					break;
				case outputtype::depth:
					internalFormat = GL_DEPTH_COMPONENT24;
					format         = GL_DEPTH_COMPONENT;
					type           = GL_UNSIGNED_BYTE;
					attachment     = GL_DEPTH_ATTACHMENT;
					break;
				case outputtype::invalid:
					log()->fatal("gl", "invalid program output type");
					break;
				}

				if(attachment == GL_COLOR_ATTACHMENT0) {
					attachment += colorAttachment++;
					drawBuffers.emplace_back(attachment);
				}

				GL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL));
				GL(glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture, 0));

				return texture;
			};

			for(auto &out : as->outs) { node.outs.emplace(out.key, fOut(out)); }
			for(auto &out : as->globalOuts) { node.globalOuts.emplace(out.key, fOut(out)); }

			if(i < nodes.size() - 1) {
				auto &nextAsset = assets[i + 1];
				auto &nextNode  = nodes[i + 1];

				for(auto &in : nextAsset->ins) {
					auto it = node.outs.find(in.key);
					if(it == node.outs.end()) {
						log()->fatal("gl", "failed to connect nodes (invalid key `" + in.key + "`)");
					}
					nextNode.ins.emplace(in.key, in.name);
				}

				for(auto &in : nextAsset->globalIns) { nextNode.globalIns.emplace(in.key, in.name); }
			}

			GL(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));

			GLenum status;
			GL(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

			std::stringstream msg;
			switch(status) {
			case GL_FRAMEBUFFER_COMPLETE:
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				log()->fatal("gl", "framebuffer unsupported");
			default:
				msg << "framebuffer status incomplete (0x" << std::hex << status << ')';
				log()->fatal("gl", msg.str());
			}
		}

		for(auto &node : nodes) {
			// upload projection matrix to pipeline stage
			project(node.program);
			// upload resolution
			uploaduniform(node.program, "u_resolution", math::point2(width, height));
		}

		for(auto uniform : uniformsU32) { setuniform(uniform.first, uniform.second, true); }
		for(auto uniform : uniformsFloat) { setuniform(uniform.first, uniform.second, true); }
		for(auto uniform : uniformsPoint3) { setuniform(uniform.first, uniform.second, true); }
	}

	GLuint gl32::makeprogram(std::shared_ptr<polar::asset::shaderprogram> as) {
		using shadertype = support::shader::shadertype;

		std::vector<GLuint> ids;
		for(auto &shader : as->shaders) {
			GLenum type = 0;
			switch(shader.type) {
			case shadertype::vertex:
				type = GL_VERTEX_SHADER;
				break;
			case shadertype::fragment:
				type = GL_FRAGMENT_SHADER;
				break;
			case shadertype::invalid:
				log()->fatal("gl", "invalid shader type");
			}

			GLuint id;
			if(!GL(id = glCreateShader(type))) { log()->fatal("gl", "failed to create shader"); }

			const GLchar *src = shader.source.c_str();
			const GLint len   = static_cast<GLint>(shader.source.length());
			if(!GL(glShaderSource(id, 1, &src, &len))) { log()->fatal("gl", "failed to upload shader source"); }
			if(!GL(glCompileShader(id))) { log()->fatal("gl", "shader compilation is unsupported on this platform"); }

			GLint status;
			if(!GL(glGetShaderiv(id, GL_COMPILE_STATUS, &status))) {
				log()->fatal("gl", "failed to get shader compilation status");
			}

			if(status == GL_FALSE) {
				GLint infoLen;
				if(!GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLen))) {
					log()->fatal("gl", "failed to get shader info log length");
				}

				if(infoLen > 0) {
					auto infoLog = std::unique_ptr<char[]>(new char[infoLen]);
					if(!GL(glGetShaderInfoLog(id, infoLen, NULL, infoLog.get()))) {
						log()->fatal("gl", "failed to get shader info log");
					}
					log()->debug("gl", "shader info log:");
					log()->debug("gl", infoLog.get());
				}
				log()->fatal("gl", "failed to compile shader");
			}

			ids.push_back(id);
		}

		GLuint programID;
		if(!GL(programID = glCreateProgram())) { log()->fatal("gl", "failed to create program"); }

		for(auto id : ids) {
			if(!GL(glAttachShader(programID, id))) { log()->fatal("gl", "failed to attach shader to program"); }

			/* flag shader for deletion */
			if(!GL(glDeleteShader(id))) { log()->fatal("gl", "failed to flag shader for deletion"); }
		}

		if(!GL(glLinkProgram(programID))) { log()->fatal("gl", "program linking is unsupported on this platform"); }

		GLint status;
		if(!GL(glGetProgramiv(programID, GL_LINK_STATUS, &status))) {
			log()->fatal("gl", "failed to get program linking status");
		}

		if(status == GL_FALSE) {
			GLint infoLen;
			if(!GL(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen))) {
				log()->fatal("gl", "failed to get program info log length");
			}

			if(infoLen > 0) {
				auto infoLog = std::unique_ptr<char[]>(new char[infoLen]);
				if(!GL(glGetProgramInfoLog(programID, infoLen, NULL, infoLog.get()))) {
					log()->fatal("gl", "failed to get program info log");
				}
				log()->debug("gl", "program info log:");
				log()->debug("gl", infoLog.get());
			}
			log()->fatal("gl", "failed to link program");
		}
		return programID;
	}

	std::shared_ptr<gl32::model_p> gl32::getpooledmodelproperty(const GLsizei) {
		if(modelPropertyPool.empty()) {
			model_p prop;

			GL(glGenVertexArrays(1, &prop.vao));
			GL(glBindVertexArray(prop.vao));

			/* location   attribute
			 *
			 *        0   vertex
			 *        1   normal
			 */

			prop.vbos.resize(1);
			GL(glGenBuffers(1, &prop.vbos[0]));

			GL(glBindBuffer(GL_ARRAY_BUFFER, prop.vbos[0]));

			const GLsizei stride = sizeof(polar::asset::vertex);
			const GLvoid *p_ptr  = NULL;
			const GLvoid *n_ptr  = (GLvoid *)sizeof(math::point3);
			const GLvoid *t_ptr  = (GLvoid *)(sizeof(math::point3) * 2);

			GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, p_ptr));
			GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, n_ptr));
			GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, t_ptr));

			GL(glEnableVertexAttribArray(0));
			GL(glEnableVertexAttribArray(1));
			GL(glEnableVertexAttribArray(2));

			return std::make_shared<model_p>(prop);
		} else {
			auto it   = modelPropertyPool.begin();
			auto prop = *it;
			modelPropertyPool.erase(it);
			return prop;
		}
	}

	void gl32::uploadmodel(std::shared_ptr<component::model> model) {
		model->generate_normals();

		GLsizei count   = GLsizei(model->asset->triangles.size()) * 3;
		GLsizeiptr size = model->asset->triangles.size() * sizeof(polar::asset::triangle);
		void *data      = model->asset->triangles.data();
		auto prop       = getpooledmodelproperty(count);

		if(count > 0) {
			if(count > prop->capacity) {
				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
				GL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW));

				prop->capacity = count;
			} else {
				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
				GL(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
			}
		}

		prop->numVertices = count;

		// textures

		auto assetM = engine->get<asset>().lock();
		if(model->asset->material) {
			auto mat    = assetM->get<polar::asset::material>(*model->asset->material);
			if(mat->diffuse_map) {
				auto diffuse_map = assetM->get<polar::asset::image>(*mat->diffuse_map);

				GL(glGenTextures(1, &prop->diffuse_map));
				GL(glBindTexture(GL_TEXTURE_2D, prop->diffuse_map));

				GLint format = GL_RGBA;
				GL(glTexImage2D(GL_TEXTURE_2D, 0, format, diffuse_map->width, diffuse_map->height, 0, format,
								GL_UNSIGNED_BYTE, diffuse_map->pixels.data()));
				GL(glGenerateMipmap(GL_TEXTURE_2D));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
			}
			if(mat->specular_map) {
				auto specular_map = assetM->get<polar::asset::image>(*mat->specular_map);

				GL(glGenTextures(1, &prop->specular_map));
				GL(glBindTexture(GL_TEXTURE_2D, prop->specular_map));

				GLint format = GL_RGBA;
				GL(glTexImage2D(GL_TEXTURE_2D, 0, format, specular_map->width, specular_map->height, 0, format,
								GL_UNSIGNED_BYTE, specular_map->pixels.data()));
				GL(glGenerateMipmap(GL_TEXTURE_2D));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
			}
			if(mat->normal_map) {
				auto normal_map = assetM->get<polar::asset::image>(*mat->normal_map);

				GL(glGenTextures(1, &prop->normal_map));
				GL(glBindTexture(GL_TEXTURE_2D, prop->normal_map));

				GLint format = GL_RGBA;
				GL(glTexImage2D(GL_TEXTURE_2D, 0, format, normal_map->width, normal_map->height, 0, format,
								GL_UNSIGNED_BYTE, normal_map->pixels.data()));
				GL(glGenerateMipmap(GL_TEXTURE_2D));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
			}
		}

		model->add<model_p>(prop);
	}

	void gl32::component_added(core::weak_ref, std::type_index ti, std::weak_ptr<component::base> ptr) {
		if(ti == typeid(component::model)) {
			auto model = std::static_pointer_cast<component::model>(ptr.lock());
			if(!model->has<model_p>()) { uploadmodel(model); }
		} else if(ti == typeid(component::sprite::base)) {
			auto sprite = std::static_pointer_cast<component::sprite::base>(ptr.lock());
			sprite->render();
			sprite_p prop;

			GL(glGenTextures(1, &prop.texture));
			GL(glBindTexture(GL_TEXTURE_2D, prop.texture));

			GLint format = GL_RGBA;
			GL(glTexImage2D(GL_TEXTURE_2D, 0, format, sprite->surface->w, sprite->surface->h, 0, format,
			                GL_UNSIGNED_BYTE, sprite->surface->pixels));
			GL(glGenerateMipmap(GL_TEXTURE_2D));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));

			sprite->add<sprite_p>(prop);
		} else if(ti == typeid(component::text)) {
			auto text   = std::static_pointer_cast<component::text>(ptr.lock());
			auto &cache = fontCache.emplace(text->as, fontcache_t()).first->second;

			for(auto c : text->str) {
				auto &entry = cache.entries[c];

				if(!entry.active) {
					auto &glyph = text->as->glyphs[c];

					GL(glGenTextures(1, &entry.texture));
					GL(glBindTexture(GL_TEXTURE_2D, entry.texture));

					GLint format = GL_RGBA;
					GL(glTexImage2D(GL_TEXTURE_2D, 0, format, glyph.surface->w, glyph.surface->h, 0, format,
					                GL_UNSIGNED_BYTE, glyph.surface->pixels));
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

	void gl32::component_removed(core::weak_ref object, std::type_index ti) {
		if(ti == typeid(component::model)) {
			auto model = engine->get<component::model>(object);
			if(model != nullptr) {
				auto prop = model->get<model_p>();
				if(prop) { modelPropertyPool.emplace(prop); }
			}
		} else if(ti == typeid(component::sprite::base)) {
			auto text = engine->get<component::sprite::base>(object);
			if(text != nullptr) {
				auto prop = text->get<sprite_p>();
				if(prop) { GL(glDeleteTextures(1, &prop->texture)); }
			}
		}
	}

	void gl32::setmousecapture(bool capture) {
		this->capture = capture;
		if(inited) {
			if(!SDL(SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE))) {
				log()->fatal("gl", "failed to set relative mouse mode");
			}
		}
	}

	void gl32::setfullscreen(bool fullscreen) {
		this->fullscreen = fullscreen;
		if(inited) {
			if(!SDL(SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))) {
				log()->critical("gl", "failed to set fullscreen mode");
			}
		}
	}

	void gl32::setdepthtest(bool depthtest) {
		if(depthtest) {
			GL(glEnable(GL_DEPTH_TEST));
		} else {
			GL(glDisable(GL_DEPTH_TEST));
		}
	}

	void gl32::setpipeline(const std::vector<std::string> &names) {
		pipelineNames = names;
		if(inited) {
			log()->trace("gl", "MakePipeline from SetPipeline");
			makepipeline(names);
			log()->trace("gl", "MakePipeline done");
		}
	}

	void gl32::setclearcolor(const math::point4 &color) { GL(glClearColor(color.r, color.g, color.b, color.a)); }

	math::decimal gl32::getuniform_decimal(const std::string &name, const math::decimal def) {
		auto it = uniformsFloat.find(name);
		if(it != uniformsFloat.end()) {
			return it->second;
		} else {
			setuniform(name, def);
			return def;
		}
	}

	math::point3 gl32::getuniform_point3(const std::string &name, const math::point3 def) {
		auto it = uniformsPoint3.find(name);
		if(it != uniformsPoint3.end()) {
			return it->second;
		} else {
			setuniform(name, def);
			return def;
		}
	}

	void gl32::setuniform(const std::string &name, glm::uint32 x, bool force) {
		auto it = uniformsU32.find(name);
		if(!force && it != uniformsU32.cend() && it->second == x) { return; }

		uniformsU32[name] = x;
		changedUniformsU32.emplace_back(name);
	}

	void gl32::setuniform(const std::string &name, math::decimal x, bool force) {
		auto it = uniformsFloat.find(name);
		if(!force && it != uniformsFloat.cend() && it->second == x) { return; }

		uniformsFloat[name] = x;
		changedUniformsFloat.emplace_back(name);
	}

	void gl32::setuniform(const std::string &name, math::point3 p, bool force) {
		auto it = uniformsPoint3.find(name);
		if(!force && it != uniformsPoint3.cend() && it->second == p) { return; }

		uniformsPoint3[name] = p;
		changedUniformsPoint3.emplace_back(name);
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, glm::int32 x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		GL(glUniform1i(loc, x));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, glm::uint32 x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		GL(glUniform1ui(loc, x));
		log()->trace("gl", "uniform ", name, " = ", x);
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, math::decimal x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto x2 = float(x);
		GL(glUniform1f(loc, x2));
		log()->trace("gl", "uniform ", name, " = ", x);
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, math::point2 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto p2 = glm::vec2(p);
		GL(glUniform2f(loc, p2.x, p2.y));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, math::point3 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto p2 = glm::vec3(p);
		GL(glUniform3f(loc, p2.x, p2.y, p2.z));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, math::point4 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto p2 = glm::vec4(p);
		GL(glUniform4f(loc, p2.x, p2.y, p2.z, p2.w));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, math::mat4x4 m) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto m2 = glm::mat4(m);
		GL(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m2)));
		return true;
	}

	math::mat4x4 gl32::calculate_projection() {
		auto heightF    = static_cast<math::decimal>(height);
		auto fovy       = 2.0f * glm::atan(heightF, math::decimal(2) * pixelDistanceFromScreen) + fovPlus;
		auto projection = glm::perspective(fovy, static_cast<math::decimal>(width) / heightF, zNear, zFar);
		// auto projection = glm::infinitePerspective(fovy,
		// static_cast<math::decimal>(width) / heightF, zNear);
		return projection;
	}

	void gl32::project(GLuint programID, math::mat4x4 proj) {
		GL(glUseProgram(programID));
		uploaduniform(programID, "u_projection", proj);
	}
} // namespace polar::system::renderer
