#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <polar/asset/shaderprogram.h>
#include <polar/component/color.h>
#include <polar/component/orientation.h>
#include <polar/component/playercamera.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/text.h>
#include <polar/core/polar.h>
#include <polar/support/action/controller.h>
#include <polar/support/action/keyboard.h>
#include <polar/support/action/mouse.h>
#include <polar/system/asset.h>
#include <polar/system/integrator.h>
#include <polar/system/renderer/gl32.h>
#include <polar/system/vr.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

namespace polar::system::renderer {
	void GLAPIENTRY debugCB(GLenum source, GLenum type, GLuint id, GLenum severity,
	                        GLsizei length, const GLchar* message,
	                        const void* userParam) {
		(void)source;
		(void)type;
		(void)id;
		(void)severity;
		(void)length;
		(void)userParam;
		debugmanager()->debug("OPENGL DEBUG OUTPUT: ", message);
	}

	bool gl32::supported() {
		gl32 renderer(nullptr, {});
		try {
			renderer.initGL();
			GLint major, minor;
			if(!GL(glGetIntegerv(GL_MAJOR_VERSION, &major))) {
				debugmanager()->fatal("failed to get OpenGL major version");
			}
			if(!GL(glGetIntegerv(GL_MINOR_VERSION, &minor))) {
				debugmanager()->fatal("failed to get OpenGL minor version");
			}
			/* if OpenGL version is 3.2 or greater */
			if(!(major > 3 || (major == 3 && minor >= 2))) {
				std::stringstream msg;
				msg << "actual OpenGL version is " << major << '.' << minor;
				debugmanager()->fatal(msg.str());
			}
			return true;
		} catch(std::exception &) { return false; }
	}

	void gl32::initGL() {
		if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) {
			debugmanager()->fatal("failed to init SDL");
		}
		if(!SDL(TTF_Init())) { debugmanager()->fatal("failed to init TTF"); }
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) {
			debugmanager()->fatal("failed to set major version attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2))) {
			debugmanager()->fatal("failed to set minor version attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
		                            SDL_GL_CONTEXT_PROFILE_CORE))) {
			debugmanager()->fatal("failed to set profile mask attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) {
			debugmanager()->fatal("failed to set double buffer attribute");
		}
		if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG))) {
			debugmanager()->fatal("failed to set context flags");
		}
		Uint32 window_flags = SDL_WINDOW_OPENGL
		                    | SDL_WINDOW_SHOWN
		                    | SDL_WINDOW_RESIZABLE
		//                    | SDL_WINDOW_ALLOW_HIGHDPI
		                    | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
		if(!SDL(window = SDL_CreateWindow(
		            "Polar Engine", SDL_WINDOWPOS_CENTERED,
		            SDL_WINDOWPOS_CENTERED, width, height, window_flags))) {
			debugmanager()->fatal("failed to create window");
		}
		if(!SDL(context = SDL_GL_CreateContext(window))) {
			debugmanager()->fatal("failed to create OpenGL context");
		}
		if(!SDL(SDL_GL_SetSwapInterval(1))) {
			debugmanager()->critical("failed to set swap interval");
		}

		if(!SDL(SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE))) {
			debugmanager()->fatal("failed to set relative mouse mode");
		}

		/* set up controller joysticks */
		SDL_GameController *controller = nullptr;
		for(int i = 0; i < SDL_NumJoysticks(); ++i) {
			bool isGameCon;
			SDL(isGameCon = SDL_IsGameController(i));
			if(isGameCon) {
				debugmanager()->verbose("SDL detected controller #", i);
				SDL(controller = SDL_GameControllerOpen(i));
			}
		}
		SDL(SDL_GameControllerEventState(SDL_ENABLE));

		glewExperimental = GL_TRUE;
		GLenum err       = glewInit();

		if(err != GLEW_OK) { debugmanager()->fatal("GLEW: glewInit failed"); }

		/* GLEW cals glGetString(EXTENSIONS) which
		 * causes GL_INVALID_ENUM on GL 3.2+ core contexts
		 */
		glGetError();

		GL(glDisable(GL_DEPTH_TEST));
		GL(glDisable(GL_BLEND));
		GL(glEnable(GL_CULL_FACE));
		GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GL(glCullFace(GL_BACK));

		if(glewIsExtensionSupported("KHR_debug")) {
			GL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
			GL(glDebugMessageCallback(debugCB, nullptr));
			GL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
			                         0, nullptr, GL_TRUE));
		}
	}

	void gl32::init() {
		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			width = vr->width();
			height = vr->height();
		}
		initGL();

		setclearcolor(Point4(0.0f));

		GL(glGenVertexArrays(1, &viewportVAO));
		GL(glBindVertexArray(viewportVAO));

		GLuint vbo;
		GL(glGenBuffers(1, &vbo));

		viewportPoints.clear();
		const float step = 0.1;
		for(float x = -1.0; x < 1.0; x += step) {
			for(float y = -1.0; y < 1.0; y += step) {
				viewportPoints.emplace_back(Point2(x, y));
				viewportPoints.emplace_back(Point2(x + step, y));
				viewportPoints.emplace_back(Point2(x, y + step));
				viewportPoints.emplace_back(Point2(x, y + step));
				viewportPoints.emplace_back(Point2(x + step, y));
				viewportPoints.emplace_back(Point2(x + step, y + step));
			}
		}

		GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * viewportPoints.size(), viewportPoints.data(),
		                GL_STATIC_DRAW));
		GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL));

		GL(glEnableVertexAttribArray(0));

		debugmanager()->trace("MakePipeline from Init");
		makepipeline(pipelineNames);
		debugmanager()->trace("MakePipeline done");

		auto assetM = engine->get<asset>().lock();
		spriteProgram =
		    makeprogram(assetM->get<polar::asset::shaderprogram>("sprite"));
		identityProgram =
		    makeprogram(assetM->get<polar::asset::shaderprogram>("identity"));

		inited = true;
	}

	void gl32::render(Mat4 proj, Mat4 view, float alpha) {
		std::unordered_map<std::string, GLuint> globals;
		for(unsigned int i = 0; i < nodes.size(); ++i) {
			auto &node = nodes[i];
			project(node.program, proj);

			GL(glBindFramebuffer(GL_FRAMEBUFFER, node.fbo));
			GL(glUseProgram(node.program));

			GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			uploaduniform(node.program, "u_view", view);
			uploaduniform(node.program, "u_invViewProj",
			              glm::inverse(proj * view));

			switch(i) {
			case 0: {
				auto pairRight =
				    engine->objects.right.equal_range(typeid(component::model));
				for(auto itRight = pairRight.first; itRight != pairRight.second;
				    ++itRight) {
					auto model =
					    static_cast<component::model *>(itRight->info.get());
					component::position *pos       = nullptr;
					component::orientation *orient = nullptr;
					component::scale *sc           = nullptr;

					auto pairLeft =
					    engine->objects.left.equal_range(itRight->get_left());
					for(auto itLeft = pairLeft.first; itLeft != pairLeft.second;
					    ++itLeft) {
						auto type = itLeft->get_right();
						if(type == typeid(component::position)) {
							pos = static_cast<component::position *>(
							    itLeft->info.get());
						} else if(type == typeid(component::orientation)) {
							orient = static_cast<component::orientation *>(
							    itLeft->info.get());
						} else if(type == typeid(component::scale)) {
							sc = static_cast<component::scale *>(
							    itLeft->info.get());
						}
					}

					auto property = model->get<model_p>().lock();
					if(property) {
						Mat4 modelMatrix(1);

						if(pos != nullptr) {
							modelMatrix = glm::translate(
							    modelMatrix,
							    pos->pos.temporal(alpha).to<Point3>());
						}
						if(orient != nullptr) {
							modelMatrix *=
							    glm::toMat4(glm::inverse(orient->orient));
						}
						if(sc != nullptr) {
							modelMatrix =
							    glm::scale(modelMatrix,
							               sc->sc.temporal(alpha).to<Point3>());
						}

						GLenum drawMode = GL_TRIANGLES;
						switch(model->type) {
						case GeometryType::Lines:
							drawMode = GL_LINES;
							break;
						case GeometryType::Triangles:
							drawMode = GL_TRIANGLES;
							break;
						case GeometryType::TriangleStrip:
							drawMode = GL_TRIANGLE_STRIP;
							break;
						case GeometryType::None:
							break;
						}

						uploaduniform(node.program, "u_model", modelMatrix);

						GL(glBindVertexArray(property->vao));
						GL(glDrawArrays(drawMode, 0, property->numVertices));
					}
				}
				break;
			}
			default:
				for(auto &pair :
				    nodes[i - 1]
				        .globalOuts) { /* for each previous global output */
					globals.emplace(pair);
				}
				unsigned int b = 0;
				for(auto &pair : node.ins) { /* for each input */
					auto buffer = nodes[i - 1].outs[pair.first];
					GL(glActiveTexture(GL_TEXTURE0 + b));
					GL(glBindTexture(GL_TEXTURE_2D, buffer));

					uploaduniform(node.program, pair.second, glm::int32(b));
					++b;
				}
				for(auto &pair : node.globalIns) { /* for each global input */
					auto buffer = globals[pair.first];
					GL(glActiveTexture(GL_TEXTURE0 + b));
					GL(glBindTexture(GL_TEXTURE_2D, buffer));

					uploaduniform(node.program, pair.second, glm::int32(b));
					++b;
				}

				GL(glBindVertexArray(viewportVAO));
				GL(glDrawArrays(GL_TRIANGLES, 0, viewportPoints.size()));

				break;
			}
		}

		// render sprites and text
		GL(glEnable(GL_BLEND));
		{
			//GL(glBindFramebuffer(GL_FRAMEBUFFER, nodes.back().fbo));
			GL(glUseProgram(spriteProgram));
			uploaduniform(spriteProgram, "u_texture", 0);
			GL(glActiveTexture(GL_TEXTURE0));
			GL(glBindVertexArray(viewportVAO));

			auto pairRight = engine->objects.right.equal_range(
			    typeid(component::sprite::base));
			for(auto itRight = pairRight.first; itRight != pairRight.second;
			    ++itRight) {
				rendersprite(itRight->get_left(), proj, view);
			}

			pairRight =
			    engine->objects.right.equal_range(typeid(component::text));
			for(auto itRight = pairRight.first; itRight != pairRight.second;
			    ++itRight) {
				rendertext(itRight->get_left(), proj, view);
			}
		}
		GL(glDisable(GL_BLEND));

		// mirror
		GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL(glUseProgram(identityProgram));
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL(glActiveTexture(GL_TEXTURE0));
		GL(glBindTexture(GL_TEXTURE_2D, nodes.back().outs.at("color")));
		uploaduniform(identityProgram, "u_colorBuffer", 0);
		GL(glBindVertexArray(viewportVAO));
		GL(glDrawArrays(GL_TRIANGLES, 0, viewportPoints.size()));
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

		fpsDtor = engine->add(fpsID);

		if(dt.Seconds() > 0) {
			fps = glm::mix(fps, 1 / dt.Seconds(), Decimal(0.1));
		}

		if(showFPS) {
			std::ostringstream oss;
			oss << (int)fps << " fps";

			auto assetM = engine->get<asset>().lock();
			auto font   = assetM->get<polar::asset::font>("nasalization-rg");

			engine->add<component::text>(fpsID, font, oss.str());
			engine->add<component::screenposition>(
			    fpsID, Point2(5, 5), support::ui::origin::topleft);
			engine->add<component::color>(fpsID, Point4(1, 1, 1, 0.8));
			engine->add<component::scale>(fpsID, Point3(0.125));
		}

		auto integrator_s = engine->get<integrator>().lock();
		float alpha       = integrator_s->alphaMicroseconds / 1000000.0f;

		Mat4 cameraView(1);
		auto pairRight =
		    engine->objects.right.equal_range(typeid(component::playercamera));
		for(auto itRight = pairRight.first; itRight != pairRight.second;
		    ++itRight) {
			auto camera =
			    static_cast<component::playercamera *>(itRight->info.get());

			component::position *pos       = nullptr;
			component::orientation *orient = nullptr;
			// component::scale *sc           = nullptr;

			auto pairLeft =
			    engine->objects.left.equal_range(itRight->get_left());
			for(auto itLeft = pairLeft.first; itLeft != pairLeft.second;
			    ++itLeft) {
				auto type = itLeft->get_right();
				if(type == typeid(component::position)) {
					pos =
					    static_cast<component::position *>(itLeft->info.get());
				} else if(type == typeid(component::orientation)) {
					orient = static_cast<component::orientation *>(
					    itLeft->info.get());
				}
			}

			cameraView = glm::translate(
			    cameraView, -camera->distance.temporal(alpha).to<Point3>());
			cameraView *= glm::toMat4(camera->orientation);
			if(orient != nullptr) { cameraView *= glm::toMat4(orient->orient); }
			cameraView = glm::translate(
			    cameraView, -camera->position.temporal(alpha).to<Point3>());
			if(pos != nullptr) {
				cameraView = glm::translate(
				    cameraView, -pos->pos.temporal(alpha).to<Point3>());
			}
		}

		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			using eye = support::vr::eye;

			vr->update_poses();

			cameraView = glm::transpose(vr->head_view()) * cameraView;

			render(vr->projection(eye::left, zNear, zFar), cameraView, alpha);
			GL(vr->submit_gl(eye::left, nodes.back().outs.at("color")));
			render(vr->projection(eye::right, zNear, zFar), cameraView, alpha);
			GL(vr->submit_gl(eye::right, nodes.back().outs.at("color")));
		} else {
			render(calculate_projection(), cameraView, alpha);
		}

		SDL(SDL_GL_SwapWindow(window));

		// handle input at beginning of frame to reduce delays in other systems
		SDL_Event event;
		while(SDL_PollEvent(&event)) { handleSDL(event); }
		SDL_ClearError();
	}

	void gl32::rendersprite(IDType id, Mat4 proj, Mat4 view) {
		using origin_t = support::ui::origin;

		auto sprite    = engine->get<component::sprite::base>(id);
		auto prop      = sprite->get<property::gl32::sprite>().lock();
		auto screenPos = engine->get<component::screenposition>(id);
		auto scale     = engine->get<component::scale>(id);
		auto color     = engine->get<component::color>(id);

		auto coord = Point2(0);

		// anchor point + local position component - alignment offset
		if(screenPos) {
			auto p = screenPos->position.get();
			switch(screenPos->origin) {
			case origin_t::bottomleft:
				coord = Point2(0, 0);
				coord += p * Point2(1, 1);
				break;
			case origin_t::bottomright:
				coord = Point2(width, 0);
				coord += p * Point2(-1, 1);
				break;
			case origin_t::topleft:
				coord = Point2(0, height);
				coord += p * Point2(1, -1);
				break;
			case origin_t::topright:
				coord = Point2(width, height);
				coord += p * Point2(-1, -1);
				break;
			case origin_t::left:
				coord = Point2(0, height / 2);
				coord += p * Point2(1, 0);
				break;
			case origin_t::right:
				coord = Point2(width, height / 2);
				coord += p * Point2(-1, 0);
				break;
			case origin_t::bottom:
				coord = Point2(width / 2, 0);
				coord += p * Point2(0, 1);
				break;
			case origin_t::top:
				coord = Point2(width / 2, height);
				coord += p * Point2(0, -1);
				break;
			case origin_t::center:
				coord = Point2(width / 2, height / 2);
				coord += p * Point2(1, 1);
				break;
			}
		}

		// offset y coord away from anchor point
		Decimal offsetY = 0;
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

		Mat4 transform(1);

		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			glm::vec3 mScale;
			glm::quat mRotation;
			glm::vec3 mTranslation;
			glm::vec3 mSkew;
			glm::vec4 mPerspective;
			glm::decompose(view, mScale, mRotation, mTranslation, mSkew, mPerspective);

			//static glm::quat rot{1, 0, 0, 0};
			//auto delta = glm::inverse(rot) * mRotation;
			//rot *= glm::pow(delta, Decimal(0.002));

			// map to plane in 3D space
			transform = proj;
			if(!sprite->fixedToViewport) { transform *= glm::inverse(glm::toMat4(mRotation)); }
			//transform = glm::rotate(transform, theta, Point3(0, 1, 0));
			transform = glm::translate(transform, Point3(0, 0, -(zNear + zFar) / 2.0));
			transform = glm::scale(transform, Point3(2.0 / width, 2.0 / height, 1));

			// scale down for VR viewing
			Decimal uiScale = 1;
			uiScale /= 8;
			transform = glm::scale(transform, Point3(uiScale, uiScale, 1));
			transform = glm::scale(transform, Point3(1, 1, (zFar - zNear) / 4.0));
		}

		// translate to bottom left corner
		transform = glm::translate(transform, Point3(-1, -1, 0));

		// normalize bounds
		transform = glm::scale(transform, Point3(2, 2, 1));

		// scale to one pixel
		transform = glm::scale(transform, Point3(1.0 / width, 1.0 / height, 1));

		// translate to screen coord
		transform = glm::translate(transform, Point3(coord.x, coord.y, 0));

		// scale by scale component
		if(scale) { transform = glm::scale(transform, scale->sc.get()); }

		uploaduniform(spriteProgram, "u_color",
		              color ? color->col.get() : Point4(1));

		// scale to sprite size
		auto sc   = Point3(sprite->surface->w, sprite->surface->h, 1);
		transform = glm::scale(transform, sc / Decimal(2));

		// translate by one sprite size
		transform = glm::translate(transform, Point3(1, offsetY, 0));

		uploaduniform(spriteProgram, "u_transform", transform);

		GL(glBindTexture(GL_TEXTURE_2D, prop->texture));
		GL(glDrawArrays(GL_TRIANGLES, 0, viewportPoints.size()));
	}

	void gl32::rendertext(IDType id, Mat4 proj, Mat4 view) {
		using origin_t = support::ui::origin;

		auto text      = engine->get<component::text>(id);
		auto screenPos = engine->get<component::screenposition>(id);
		auto scale     = engine->get<component::scale>(id);
		auto color     = engine->get<component::color>(id);

		// calculate intended width of string
		Decimal stringWidth = 0;
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

		auto coord = Point2(0);

		// anchor point + local position component - alignment offset
		if(screenPos) {
			auto p = screenPos->position.get();
			switch(screenPos->origin) {
			case origin_t::bottomleft:
				coord = Point2(0, 0);
				coord += p * Point2(1, 1);
				coord.x -= 0;
				break;
			case origin_t::bottomright:
				coord = Point2(width, 0);
				coord += p * Point2(-1, 1);
				coord.x -= stringWidth;
				break;
			case origin_t::topleft:
				coord = Point2(0, height);
				coord += p * Point2(1, -1);
				coord.x -= 0;
				break;
			case origin_t::topright:
				coord = Point2(width, height);
				coord += p * Point2(-1, -1);
				coord.x -= stringWidth;
				break;
			case origin_t::left:
				coord = Point2(0, height / 2);
				coord += p * Point2(1, 0);
				coord.x -= 0;
				break;
			case origin_t::right:
				coord = Point2(width, height / 2);
				coord += p * Point2(-1, 0);
				coord.x -= stringWidth;
				break;
			case origin_t::bottom:
				coord = Point2(width / 2, 0);
				coord += p * Point2(0, 1);
				coord.x -= stringWidth / 2;
				break;
			case origin_t::top:
				coord = Point2(width / 2, height);
				coord += p * Point2(0, -1);
				coord.x -= stringWidth / 2;
				break;
			case origin_t::center:
				coord = Point2(width / 2, height / 2);
				coord += p * Point2(1, 1);
				coord.x -= stringWidth / 2;
				break;
			}
		}

		// offset y coord away from anchor point
		Decimal offsetY = 0;
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

		Mat4 transform(1);

		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			glm::vec3 mScale;
			glm::quat mRotation;
			glm::vec3 mTranslation;
			glm::vec3 mSkew;
			glm::vec4 mPerspective;
			glm::decompose(view, mScale, mRotation, mTranslation, mSkew, mPerspective);

			//static glm::quat rot{1, 0, 0, 0};
			//auto delta = glm::inverse(rot) * mRotation;
			//rot *= glm::pow(delta, Decimal(0.002));

			// map to plane in 3D space
			transform = proj;
			if(!text->fixedToViewport) { transform *= glm::inverse(glm::toMat4(mRotation)); }
			//transform = glm::rotate(transform, theta, Point3(0, 1, 0));
			transform = glm::translate(transform, Point3(0, 0, -(zNear + zFar) / 2.0));
			transform = glm::scale(transform, Point3(2.0 / width, 2.0 / height, 1));

			// scale down for VR viewing
			Decimal uiScale = 1;
			uiScale /= 8;
			transform = glm::scale(transform, Point3(uiScale, uiScale, 1));
			transform = glm::scale(transform, Point3(1, 1, (zFar - zNear) / 4.0));
		}

		// translate to bottom left corner
		transform = glm::translate(transform, Point3(-1, -1, 0));

		// normalize bounds
		transform = glm::scale(transform, Point3(2, 2, 1));

		// scale to one pixel
		transform = glm::scale(transform, Point3(1.0 / width, 1.0 / height, 1));

		// translate to screen coord
		transform = glm::translate(transform, Point3(coord.x, coord.y, 0));

		// scale by scale component
		if(scale) { transform = glm::scale(transform, scale->sc.get()); }

		uploaduniform(spriteProgram, "u_color",
		              color ? color->col.get() : Point4(1));

		Decimal pen = 0;
		for(auto c : text->str) {
			Mat4 glyphTransform = transform;

			// translate by glyph pen
			glyphTransform = glm::translate(glyphTransform, Point3(pen, 0, 0));

			// translate by glyph min x
			glyphTransform = glm::translate(
			    glyphTransform, Point3(text->as->glyphs[c].min.x, 0, 0));

			// scale to glyph size
			auto sc        = Point3(text->as->glyphs[c].surface->w,
                             text->as->glyphs[c].surface->h, 1);
			glyphTransform = glm::scale(glyphTransform, sc / Decimal(2));

			// translate by one glyph size
			glyphTransform =
			    glm::translate(glyphTransform, Point3(1, offsetY, 0));

			uploaduniform(spriteProgram, "u_transform", glyphTransform);

			GL(glBindTexture(GL_TEXTURE_2D,
			                 fontCache[text->as].entries[c].texture));
			GL(glDrawArrays(GL_TRIANGLES, 0, viewportPoints.size()));

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
					width = vr->width();
					height = vr->height();
				} else {
					int w, h;
					SDL(SDL_GL_GetDrawableSize(window, &w, &h));
					width = w;
					height = h;
				}

				GL(glViewport(0, 0, width, height));
				debugmanager()->trace("MakePipeline from SDL_WINDOWEVENT");
				makepipeline(pipelineNames);
				debugmanager()->trace("MakePipeline done");
				if(act) {
					act->trigger<action_resize>();
				}
				break;
			}
			break;
		case SDL_KEYDOWN:
			if(ev.key.repeat == 0) {
				key = mkKeyFromSDL(ev.key.keysym.sym);
				if(act) {
					act->trigger_digital(kb::key_ti(key), true);
				}
			}
			break;
		case SDL_KEYUP:
			key = mkKeyFromSDL(ev.key.keysym.sym);
			if(act) {
				act->trigger_digital(kb::key_ti(key), false);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			key = mkMouseButtonFromSDL(ev.button.button);
			if(act) {
				act->trigger_digital(kb::key_ti(key), true);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			key = mkMouseButtonFromSDL(ev.button.button);
			if(act) {
				act->trigger_digital(kb::key_ti(key), false);
			}
			break;
		case SDL_MOUSEMOTION:
			if(act) {
				// XXX: this is hacky
				act->accumulate<mouse::position_x>(ev.motion.x);
				act->accumulate<mouse::position_y>(ev.motion.y);

				act->accumulate<mouse::motion_x>(ev.motion.xrel);
				act->accumulate<mouse::motion_y>(ev.motion.yrel);
			}
			break;
		case SDL_MOUSEWHEEL:
			if(act) {
				act->accumulate<mouse::wheel_x>(ev.wheel.x);
				act->accumulate<mouse::wheel_y>(ev.wheel.y);
			}
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			key = mkButtonFromSDL(
			    static_cast<SDL_GameControllerButton>(ev.cbutton.button));
			if(act) {
				act->trigger_digital(kb::key_ti(key), true);
			}
			break;
		case SDL_CONTROLLERBUTTONUP:
			key = mkButtonFromSDL(
			    static_cast<SDL_GameControllerButton>(ev.cbutton.button));
			if(act) {
				act->trigger_digital(kb::key_ti(key), false);
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			switch(ev.caxis.axis) {
			case 0: // x axis
				if(act) {
					act->accumulate<controller::motion_x>(ev.caxis.value);
				}
				break;
			case 1: // y axis
				if(act) {
					act->accumulate<controller::motion_y>(ev.caxis.value);
				}
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
			for(auto &out : node.globalOuts) {
				GL(glDeleteTextures(1, &out.second));
			}
			for(auto &out : node.outs) { GL(glDeleteTextures(1, &out.second)); }
			GL(glDeleteFramebuffers(1, &node.fbo));
			GL(glDeleteProgram(node.program));
		}
		nodes.clear();

		for(auto &name : names) {
			debugmanager()->verbose("building shader program `", name, '`');
			auto as = assetM->get<polar::asset::shaderprogram>(name);
			assets.emplace_back(as);
			nodes.emplace_back(makeprogram(as));
			for(auto &uniform : as->uniforms) {
				nodes.back().uniforms.emplace(uniform);
			}
		}

		for(unsigned int i = 0; i < nodes.size(); ++i) {
			auto &as = assets[i];
			auto &node = nodes[i];

			std::vector<GLenum> drawBuffers;
			int colorAttachment = 0;

			GL(glGenFramebuffers(1, &node.fbo));
			GL(glBindFramebuffer(GL_FRAMEBUFFER, node.fbo));

			auto fOut = [this, &drawBuffers,
			             &colorAttachment](polar::asset::shaderoutput &out) {
				using outputtype = support::shader::outputtype;

				GLuint texture;
				GL(glGenTextures(1, &texture));
				GL(glBindTexture(GL_TEXTURE_2D, texture));

				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				                   GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				                   GL_CLAMP_TO_EDGE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				                   GL_NEAREST));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				                   GL_NEAREST));

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
					debugmanager()->fatal("invalid program output type");
					break;
				}

				if(attachment == GL_COLOR_ATTACHMENT0) {
					attachment += colorAttachment++;
					drawBuffers.emplace_back(attachment);
				}

				GL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
				                0, format, type, NULL));
				GL(glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture,
				                        0));

				return texture;
			};

			for(auto &out : as->outs) { node.outs.emplace(out.key, fOut(out)); }
			for(auto &out : as->globalOuts) {
				node.globalOuts.emplace(out.key, fOut(out));
			}

			if(i < nodes.size() - 1) {
				auto &nextAsset = assets[i + 1];
				auto &nextNode = nodes[i + 1];

				for(auto &in : nextAsset->ins) {
					auto it = node.outs.find(in.key);
					if(it == node.outs.end()) {
						debugmanager()->fatal(
							"failed to connect nodes (invalid key `" + in.key +
							"`)");
					}
					nextNode.ins.emplace(in.key, in.name);
				}

				for(auto &in : nextAsset->globalIns) {
					nextNode.globalIns.emplace(in.key, in.name);
				}
			}

			GL(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()),
			                 drawBuffers.data()));

			GLenum status;
			GL(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

			std::stringstream msg;
			switch(status) {
			case GL_FRAMEBUFFER_COMPLETE:
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				debugmanager()->fatal("framebuffer unsupported");
			default:
				msg << "framebuffer status incomplete (0x" << std::hex << status
				    << ')';
				debugmanager()->fatal(msg.str());
			}
		}

		for(auto &node : nodes) {
			// upload projection matrix to pipeline stage
			project(node.program);
			// upload resolution
			uploaduniform(node.program, "u_resolution", Point2(width, height));
		}

		for(auto uniform : uniformsU32) {
			setuniform(uniform.first, uniform.second, true);
		}
		for(auto uniform : uniformsFloat) {
			setuniform(uniform.first, uniform.second, true);
		}
		for(auto uniform : uniformsPoint3) {
			setuniform(uniform.first, uniform.second, true);
		}
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
				debugmanager()->fatal("invalid shader type");
			}

			GLuint id;
			if(!GL(id = glCreateShader(type))) {
				debugmanager()->fatal("failed to create shader");
			}

			const GLchar *src = shader.source.c_str();
			const GLint len   = static_cast<GLint>(shader.source.length());
			if(!GL(glShaderSource(id, 1, &src, &len))) {
				debugmanager()->fatal("failed to upload shader source");
			}
			if(!GL(glCompileShader(id))) {
				debugmanager()->fatal(
				    "shader compilation is unsupported on this platform");
			}

			GLint status;
			if(!GL(glGetShaderiv(id, GL_COMPILE_STATUS, &status))) {
				debugmanager()->fatal(
				    "failed to get shader compilation status");
			}

			if(status == GL_FALSE) {
				GLint infoLen;
				if(!GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLen))) {
					debugmanager()->fatal(
					    "failed to get shader info log length");
				}

				if(infoLen > 0) {
					auto infoLog = std::unique_ptr<char[]>(new char[infoLen]);
					if(!GL(glGetShaderInfoLog(id, infoLen, NULL,
					                          infoLog.get()))) {
						debugmanager()->fatal("failed to get shader info log");
					}
					debugmanager()->debug("shader info log:");
					debugmanager()->debug(infoLog.get());
				}
				debugmanager()->fatal("failed to compile shader");
			}

			ids.push_back(id);
		}

		GLuint programID;
		if(!GL(programID = glCreateProgram())) {
			debugmanager()->fatal("failed to create program");
		}

		for(auto id : ids) {
			if(!GL(glAttachShader(programID, id))) {
				debugmanager()->fatal("failed to attach shader to program");
			}

			/* flag shader for deletion */
			if(!GL(glDeleteShader(id))) {
				debugmanager()->fatal("failed to flag shader for deletion");
			}
		}

		if(!GL(glLinkProgram(programID))) {
			debugmanager()->fatal(
			    "program linking is unsupported on this platform");
		}

		GLint status;
		if(!GL(glGetProgramiv(programID, GL_LINK_STATUS, &status))) {
			debugmanager()->fatal("failed to get program linking status");
		}

		if(status == GL_FALSE) {
			GLint infoLen;
			if(!GL(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen))) {
				debugmanager()->fatal("failed to get program info log length");
			}

			if(infoLen > 0) {
				auto infoLog = std::unique_ptr<char[]>(new char[infoLen]);
				if(!GL(glGetProgramInfoLog(programID, infoLen, NULL,
				                           infoLog.get()))) {
					debugmanager()->fatal("failed to get program info log");
				}
				debugmanager()->debug("program info log:");
				debugmanager()->debug(infoLog.get());
			}
			debugmanager()->fatal("failed to link program");
		}
		return programID;
	}

	std::shared_ptr<gl32::model_p>
	gl32::getpooledmodelproperty(const GLsizei) {
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
			auto it = modelPropertyPool.begin();
			auto prop = *it;
			modelPropertyPool.erase(it);
			return prop;
		}
	}

	void gl32::uploadmodel(std::shared_ptr<component::model> model) {
		auto normals     = model->calculate_normals();
		auto numVertices = normals.size();
		auto dataSize    = sizeof(component::model::point_t) * numVertices;
		auto prop        = getpooledmodelproperty((GLsizei)numVertices);

		if(numVertices > 0) {
			if(GLsizei(numVertices) > prop->capacity) {
				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
				GL(glBufferData(GL_ARRAY_BUFFER, dataSize, model->points.data(),
				                GL_DYNAMIC_DRAW));

				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
				GL(glBufferData(GL_ARRAY_BUFFER, dataSize, normals.data(),
				                GL_DYNAMIC_DRAW));

				prop->capacity = (GLsizei)numVertices;
			} else {
				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
				GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize,
				                   model->points.data()));

				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
				GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize,
				                   normals.data()));
			}
		}

		prop->numVertices = (GLsizei)numVertices;

		model->add<model_p>(prop);
		// model->points.clear();
		// model->points.shrink_to_fit();
	}

	void gl32::componentadded(IDType, std::type_index ti,
	                          std::weak_ptr<component::base> ptr) {
		if(ti == typeid(component::model)) {
			auto model = std::static_pointer_cast<component::model>(ptr.lock());
			if(!model->has<model_p>()) { uploadmodel(model); }
		} else if(ti == typeid(component::sprite::base)) {
			auto sprite =
			    std::static_pointer_cast<component::sprite::base>(ptr.lock());
			sprite->render();
			sprite_p prop;

			GL(glGenTextures(1, &prop.texture));
			GL(glBindTexture(GL_TEXTURE_2D, prop.texture));

			GLint format = GL_RGBA;
			GL(glTexImage2D(GL_TEXTURE_2D, 0, format, sprite->surface->w,
			                sprite->surface->h, 0, format, GL_UNSIGNED_BYTE,
			                sprite->surface->pixels));
			GL(glGenerateMipmap(GL_TEXTURE_2D));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			                   GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			                   GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			                   GL_LINEAR));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			                   GL_LINEAR_MIPMAP_NEAREST));

			sprite->add<sprite_p>(prop);
		} else if(ti == typeid(component::text)) {
			auto text = std::static_pointer_cast<component::text>(ptr.lock());
			auto &cache =
			    fontCache.emplace(text->as, fontcache_t()).first->second;

			for(auto c : text->str) {
				auto &entry = cache.entries[c];

				if(!entry.active) {
					auto &glyph = text->as->glyphs[c];

					GL(glGenTextures(1, &entry.texture));
					GL(glBindTexture(GL_TEXTURE_2D, entry.texture));

					GLint format = GL_RGBA;
					GL(glTexImage2D(GL_TEXTURE_2D, 0, format, glyph.surface->w,
					                glyph.surface->h, 0, format,
					                GL_UNSIGNED_BYTE, glyph.surface->pixels));
					GL(glGenerateMipmap(GL_TEXTURE_2D));
					GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					                   GL_CLAMP_TO_EDGE));
					GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					                   GL_CLAMP_TO_EDGE));
					GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					                   GL_LINEAR));
					GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					                   GL_LINEAR_MIPMAP_NEAREST));

					entry.active = true;
				}
			}
		}
	}

	void gl32::componentremoved(IDType id, std::type_index ti) {
		if(ti == typeid(component::model)) {
			auto model = engine->get<component::model>(id);
			if(model != nullptr) {
				auto prop = model->get<model_p>().lock();
				if(prop) { modelPropertyPool.emplace(prop); }
			}
		} else if(ti == typeid(component::sprite::base)) {
			auto text = engine->get<component::sprite::base>(id);
			if(text != nullptr) {
				auto prop = text->get<sprite_p>().lock();
				if(prop) { GL(glDeleteTextures(1, &prop->texture)); }
			}
		}
	}

	void gl32::setmousecapture(bool capture) {
		this->capture = capture;
		if(inited) {
			if(!SDL(SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE))) {
				debugmanager()->fatal("failed to set relative mouse mode");
			}
		}
	}

	void gl32::setfullscreen(bool fullscreen) {
		this->fullscreen = fullscreen;
		if(inited) {
			if(!SDL(SDL_SetWindowFullscreen(
			       window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))) {
				debugmanager()->critical("failed to set fullscreen mode");
			}
		}
	}

	void gl32::setpipeline(const std::vector<std::string> &names) {
		pipelineNames = names;
		if(inited) {
			debugmanager()->trace("MakePipeline from SetPipeline");
			makepipeline(names);
			debugmanager()->trace("MakePipeline done");
		}
	}

	void gl32::setclearcolor(const Point4 &color) {
		GL(glClearColor(color.r, color.g, color.b, color.a));
	}

	Decimal gl32::getuniform_decimal(const std::string &name,
	                                 const Decimal def) {
		auto it = uniformsFloat.find(name);
		if(it != uniformsFloat.end()) {
			return it->second;
		} else {
			setuniform(name, def);
			return def;
		}
	}

	Point3 gl32::getuniform_point3(const std::string &name, const Point3 def) {
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

	void gl32::setuniform(const std::string &name, Decimal x, bool force) {
		auto it = uniformsFloat.find(name);
		if(!force && it != uniformsFloat.cend() && it->second == x) { return; }

		uniformsFloat[name] = x;
		changedUniformsFloat.emplace_back(name);
	}

	void gl32::setuniform(const std::string &name, Point3 p, bool force) {
		auto it = uniformsPoint3.find(name);
		if(!force && it != uniformsPoint3.cend() && it->second == p) { return; }

		uniformsPoint3[name] = p;
		changedUniformsPoint3.emplace_back(name);
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name,
	                         glm::int32 x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) {
			return false;
		} // -1 if uniform does not exist in program
		GL(glUniform1i(loc, x));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name,
	                         glm::uint32 x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) {
			return false;
		} // -1 if uniform does not exist in program
		GL(glUniform1ui(loc, x));
		debugmanager()->trace("uniform ", name, " = ", x);
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name,
	                         Decimal x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) {
			return false;
		} // -1 if uniform does not exist in program
		auto x2 = float(x);
		GL(glUniform1f(loc, x2));
		debugmanager()->trace("uniform ", name, " = ", x);
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name,
	                         Point2 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) {
			return false;
		} // -1 if uniform does not exist in program
		auto p2 = glm::vec2(p);
		GL(glUniform2f(loc, p2.x, p2.y));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name,
	                         Point3 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) {
			return false;
		} // -1 if uniform does not exist in program
		auto p2 = glm::vec3(p);
		GL(glUniform3f(loc, p2.x, p2.y, p2.z));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name,
	                         Point4 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) {
			return false;
		} // -1 if uniform does not exist in program
		auto p2 = glm::vec4(p);
		GL(glUniform4f(loc, p2.x, p2.y, p2.z, p2.w));
		return true;
	}

	bool gl32::uploaduniform(GLuint program, const std::string &name, Mat4 m) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) {
			return false;
		} // -1 if uniform does not exist in program
		auto m2 = glm::mat4(m);
		GL(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m2)));
		return true;
	}

	Mat4 gl32::calculate_projection() {
		auto heightF = static_cast<Decimal>(height);
		auto fovy =
		    2.0f * glm::atan(heightF, Decimal(2) * pixelDistanceFromScreen) +
		    fovPlus;
		auto projection = glm::perspective(
		    fovy, static_cast<Decimal>(width) / heightF, zNear, zFar);
		// auto projection = glm::infinitePerspective(fovy,
		// static_cast<Decimal>(width) / heightF, zNear);
		return projection;
	}

	void gl32::project(GLuint programID, Mat4 proj) {
		GL(glUseProgram(programID));
		uploaduniform(programID, "u_projection", proj);
	}
} // namespace polar::system::renderer
