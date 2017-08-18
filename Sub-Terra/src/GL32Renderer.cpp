#include "common.h"
#include "GL32Renderer.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "Integrator.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "PlayerCameraComponent.h"
#include "ScaleComponent.h"
#include "ColorComponent.h"
#include "Text.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

bool GL32Renderer::IsSupported() {
	GL32Renderer renderer(nullptr, {});
	try {
		renderer.InitGL();
		GLint major, minor;
		if(!GL(glGetIntegerv(GL_MAJOR_VERSION, &major))) { DebugManager()->Fatal("failed to get OpenGL major version"); }
		if(!GL(glGetIntegerv(GL_MINOR_VERSION, &minor))) { DebugManager()->Fatal("failed to get OpenGL minor version"); }
		/* if OpenGL version is 3.2 or greater */
 		if(!(major > 3 || (major == 3 && minor >= 2))) {
			std::stringstream msg;
			msg << "actual OpenGL version is " << major << '.' << minor;
			DebugManager()->Fatal(msg.str());
		}
		return true;
	} catch(std::exception &) {
		return false;
	}
}

void GL32Renderer::InitGL() {
	if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) { DebugManager()->Fatal("failed to init SDL"); }
	if(!SDL(TTF_Init())) { DebugManager()->Fatal("failed to init TTF"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) { DebugManager()->Fatal("failed to set major version attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2))) { DebugManager()->Fatal("failed to set minor version attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) { DebugManager()->Fatal("failed to set profile mask attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) { DebugManager()->Fatal("failed to set double buffer attribute"); }
	if(!SDL(window = SDL_CreateWindow("Polar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)))) {
		DebugManager()->Fatal("failed to create window");
	}
	if(!SDL(context = SDL_GL_CreateContext(window))) { DebugManager()->Fatal("failed to create OpenGL context"); }
	if(!SDL(SDL_GL_SetSwapInterval(1))) { DebugManager()->Fatal("failed to set swap interval"); }

	if(!SDL(SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE))) { DebugManager()->Fatal("failed to set relative mouse mode"); }

	/* set up controller joysticks */
	SDL_GameController *controller = nullptr;
	for(int i = 0; i < SDL_NumJoysticks(); ++i) {
		bool isGameCon;
		SDL(isGameCon = SDL_IsGameController(i));
		if(isGameCon) {
			DebugManager()->Verbose("SDL detected controller #", i);
			SDL(controller = SDL_GameControllerOpen(i));
		}
	}
	SDL(SDL_GameControllerEventState(SDL_ENABLE));

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if(err != GLEW_OK) { DebugManager()->Fatal("GLEW: glewInit failed"); }

	/* GLEW cals glGetString(EXTENSIONS) which
	 * causes GL_INVALID_ENUM on GL 3.2+ core contexts
	 */
	glGetError();

	GL(glDisable(GL_DEPTH_TEST));
	GL(glEnable(GL_BLEND));
	GL(glEnable(GL_CULL_FACE));
	GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL(glCullFace(GL_BACK));
}

void GL32Renderer::Init() {
	InitGL();

	SetClearColor(Point4(0.0f));

	GL(glGenVertexArrays(1, &viewportVAO));
	GL(glBindVertexArray(viewportVAO));

	GLuint vbo;
	GL(glGenBuffers(1, &vbo));

	std::vector<glm::vec2> points = {
		Point2(-1, -1),
		Point2( 1, -1),
		Point2(-1,  1),
		Point2( 1,  1)
	};

	GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, points.data(), GL_STATIC_DRAW));
	GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL));

	GL(glEnableVertexAttribArray(0));

	DebugManager()->Trace("MakePipeline from Init");
	MakePipeline(pipelineNames);
	DebugManager()->Trace("MakePipeline done");

	auto assetM = engine->GetSystem<AssetManager>().lock();
	spriteProgram = MakeProgram(assetM->Get<ShaderProgramAsset>("sprite"));

	inited = true;
}

void GL32Renderer::Update(DeltaTicks &dt) {
	// upload changed uniforms
	for(auto &node : nodes) {
		GL(glUseProgram(node.program));
		for(auto &name : changedUniformsU32) {
			if(node.uniforms.find(name) != node.uniforms.cend()) {
				UploadUniform(node.program, name, uniformsU32[name]);
			}
		}
		for(auto &name : changedUniformsFloat) {
			if(node.uniforms.find(name) != node.uniforms.cend()) {
				UploadUniform(node.program, name, uniformsFloat[name]);
			}
		}
		for(auto &name : changedUniformsPoint3) {
			if(node.uniforms.find(name) != node.uniforms.cend()) {
				UploadUniform(node.program, name, uniformsPoint3[name]);
			}
		}
		changedUniformsU32.clear();
		changedUniformsFloat.clear();
		changedUniformsPoint3.clear();
	}

	fpsDtor = engine->AddObject(&fpsID);

	if(dt.Seconds() > 0) {
		fps = glm::mix(fps, 1 / dt.Seconds(), Decimal(0.1));
	}

	if(showFPS) {
		std::ostringstream oss;
		oss << (int)fps << " fps";

		auto assetM = engine->GetSystem<AssetManager>().lock();
		auto font = assetM->Get<FontAsset>("nasalization-rg");

		engine->AddComponent<Text>(fpsID, font, oss.str());
		engine->AddComponent<ScreenPositionComponent>(fpsID, Point2(5, 5), Origin::TopLeft);
		engine->AddComponent<ColorComponent>(fpsID, Point4(1, 1, 1, 0.8));
		engine->AddComponent<ScaleComponent>(fpsID, Point3(0.125));
	}

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		HandleSDL(event);
	}
	SDL_ClearError();

	auto integrator = engine->GetSystem<Integrator>().lock();
	float alpha = integrator->alphaMicroseconds / 1000000.0f;

	Mat4 cameraView;
	auto pairRight = engine->objects.right.equal_range(&typeid(PlayerCameraComponent));
	for(auto itRight = pairRight.first; itRight != pairRight.second; ++itRight) {
		auto camera = static_cast<PlayerCameraComponent *>(itRight->info.get());

		PositionComponent *pos = nullptr;
		OrientationComponent *orient = nullptr;

		auto pairLeft = engine->objects.left.equal_range(itRight->get_left());
		for(auto itLeft = pairLeft.first; itLeft != pairLeft.second; ++itLeft) {
			auto type = itLeft->get_right();
			if(type == &typeid(PositionComponent)) { pos = static_cast<PositionComponent *>(itLeft->info.get()); } else if(type == &typeid(OrientationComponent)) { orient = static_cast<OrientationComponent *>(itLeft->info.get()); }
		}

		cameraView = glm::translate(cameraView, -camera->distance.Temporal(alpha).To<Point3>());
		cameraView *= glm::toMat4(camera->orientation);
		if(orient != nullptr) { cameraView *= glm::toMat4(orient->orientation); }
		cameraView = glm::translate(cameraView, -camera->position.Temporal(alpha).To<Point3>());
		if(pos != nullptr) { cameraView = glm::translate(cameraView, -pos->position.Temporal(alpha).To<Point3>()); }
	}

	std::unordered_map<std::string, GLuint> globals;
	for(unsigned int i = 0; i < nodes.size(); ++i) {
		auto &node = nodes[i];

		GL(glBindFramebuffer(GL_FRAMEBUFFER, node.fbo));
		GL(glUseProgram(node.program));
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		UploadUniform(node.program, "u_view", cameraView);
		UploadUniform(node.program, "u_invViewProj", glm::inverse(CalculateProjection() * cameraView));

		switch(i) {
		case 0: {
			// freefall main shader is in screen space
			GL(glBindVertexArray(viewportVAO));
			GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
			break;

			/*auto pairRight = engine->objects.right.equal_range(&typeid(ModelComponent));
			for(auto itRight = pairRight.first; itRight != pairRight.second; ++itRight) {
				auto model = static_cast<ModelComponent *>(itRight->info.get());
				Mat4 modelMatrix;

				PositionComponent *pos = nullptr;
				OrientationComponent *orient = nullptr;

				auto pairLeft = engine->objects.left.equal_range(itRight->get_left());
				for(auto itLeft = pairLeft.first; itLeft != pairLeft.second; ++itLeft) {
					auto type = itLeft->get_right();
					if(type == &typeid(PositionComponent)) { pos = static_cast<PositionComponent *>(itLeft->info.get()); } else if(type == &typeid(OrientationComponent)) { orient = static_cast<OrientationComponent *>(itLeft->info.get()); }
				}

				auto property = model->Get<GL32ModelProperty>().lock();
				if(property) {
					glm::dmat4 modelView = cameraView;

					if(pos != nullptr) { modelMatrix = glm::translate(modelMatrix, pos->position.Temporal(alpha).To<Point3>()); }
					if(orient != nullptr) { modelMatrix *= glm::toMat4(glm::inverse(orient->orientation)); }

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

					UploadUniform(node.program, "u_model", modelMatrix);

					GL(glBindVertexArray(property->vao));
					GL(glDrawArrays(drawMode, 0, property->numVertices));
				}
			}
			break;*/
		}
		default:
			for(auto &pair : nodes[i - 1].globalOuts) { /* for each previous global output */
				globals.emplace(pair);
			}
			unsigned int b = 0;
			for(auto &pair : node.ins) { /* for each input */
				auto buffer = nodes[i - 1].outs[pair.first];
				GL(glActiveTexture(GL_TEXTURE0 + b));
				GL(glBindTexture(GL_TEXTURE_2D, buffer));

				UploadUniform(node.program, pair.second.c_str(), glm::int32(b));
				++b;
			}
			for(auto &pair : node.globalIns) { /* for each global input*/
				auto buffer = globals[pair.first];
				GL(glActiveTexture(GL_TEXTURE0 + b));
				GL(glBindTexture(GL_TEXTURE_2D, buffer));

				UploadUniform(node.program, pair.second.c_str(), glm::int32(b));
				++b;
			}

			GL(glBindVertexArray(viewportVAO));
			GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

			break;
		}
	}

	// sprites
	{
		GL(glUseProgram(spriteProgram));
		UploadUniform(spriteProgram, "u_texture", 0);
		GL(glActiveTexture(GL_TEXTURE0));
		GL(glBindVertexArray(viewportVAO));

		auto pairRight = engine->objects.right.equal_range(&typeid(Sprite));
		for(auto itRight = pairRight.first; itRight != pairRight.second; ++itRight) {
			// XXX
		}

		pairRight = engine->objects.right.equal_range(&typeid(Text));
		for(auto itRight = pairRight.first; itRight != pairRight.second; ++itRight) {
			RenderText(itRight->get_left());
		}
	}

	SDL(SDL_GL_SwapWindow(window));
}

void GL32Renderer::RenderText(IDType id) {
	auto text = engine->GetComponent<Text>(id);
	auto screenPos = engine->GetComponent<ScreenPositionComponent>(id);
	auto scale = engine->GetComponent<ScaleComponent>(id);
	auto color = engine->GetComponent<ColorComponent>(id);

	// calculate intended width of string
	Decimal stringWidth = 0;
	for(auto c : text->str) {
		stringWidth += text->asset->glyphs[c].advance;
	}

	// add required padding to string width
	if(screenPos) {
		auto &last = text->asset->glyphs[text->str.back()];
		switch(screenPos->origin) {
		case Origin::BottomRight:
		case Origin::Right:
		case Origin::TopRight:
			stringWidth += last.max.x / 2;
			break;
		case Origin::Bottom:
		case Origin::Top:
		case Origin::Center:
			stringWidth += last.max.x / 4;
			break;
		}
	}

	// scale string width by scale component
	if(scale) {
		stringWidth *= scale->scale.Get().x;
	}

	auto coord = Point2(0);

	// anchor point + local position component - alignment offset
	if(screenPos) {
		auto p = screenPos->position.Get();
		switch(screenPos->origin) {
		case Origin::BottomLeft:
			coord = Point2(0, 0);
			coord += p * Point2(1, 1);
			coord.x -= 0;
			break;
		case Origin::BottomRight:
			coord = Point2(width, 0);
			coord += p * Point2(-1, 1);
			coord.x -= stringWidth;
			break;
		case Origin::TopLeft:
			coord = Point2(0, height);
			coord += p * Point2(1, -1);
			coord.x -= 0;
			break;
		case Origin::TopRight:
			coord = Point2(width, height);
			coord += p * Point2(-1, -1);
			coord.x -= stringWidth;
			break;
		case Origin::Left:
			coord = Point2(0, height / 2);
			coord += p * Point2(1, 0);
			coord.x -= 0;
			break;
		case Origin::Right:
			coord = Point2(width, height / 2);
			coord += p * Point2(-1, 0);
			coord.x -= stringWidth;
			break;
		case Origin::Bottom:
			coord = Point2(width / 2, 0);
			coord += p * Point2(0, 1);
			coord.x -= stringWidth / 2;
			break;
		case Origin::Top:
			coord = Point2(width / 2, height);
			coord += p * Point2(0, -1);
			coord.x -= stringWidth / 2;
			break;
		case Origin::Center:
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
		case Origin::TopLeft:
		case Origin::Top:
		case Origin::TopRight:
			offsetY = -1;
			break;
		case Origin::BottomLeft:
		case Origin::Bottom:
		case Origin::BottomRight:
			offsetY = 1;
			break;
		}
	}

	Mat4 transform;

	// translate to bottom left corner
	transform = glm::translate(transform, Point3(-1, -1, 0));

	// scale to one pixel
	transform = glm::scale(transform, Decimal(2) / Point3(width, height, 1));

	// translate to screen coord
	transform = glm::translate(transform, Point3(coord, 0));

	// scale by scale component
	if(scale) {
		transform = glm::scale(transform, scale->scale.Get());
	}

	UploadUniform(spriteProgram, "u_color", color ? color->color.Get() : Point4(1));

	Decimal pen = 0;
	for(auto c : text->str) {
		Mat4 glyphTransform = transform;

		// translate by glyph pen
		glyphTransform = glm::translate(glyphTransform, Point3(pen, 0, 0));

		// translate by glyph min x
		glyphTransform = glm::translate(glyphTransform, Point3(text->asset->glyphs[c].min.x, 0, 0));

		// scale to glyph size
		auto sc = Point3(text->asset->glyphs[c].surface->w, text->asset->glyphs[c].surface->h, 1);
		glyphTransform = glm::scale(glyphTransform, sc / Decimal(2));

		// translate by one glyph size
		glyphTransform = glm::translate(glyphTransform, Point3(1, offsetY, 0));

		UploadUniform(spriteProgram, "u_transform", glyphTransform);

		GL(glBindTexture(GL_TEXTURE_2D, fontCache[text->asset].entries[c].texture));
		GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		pen += text->asset->glyphs[c].advance;
	}
}

GL32Renderer::~GL32Renderer() {
	SDL(SDL_GL_DeleteContext(context));
	SDL(SDL_DestroyWindow(window));
	SDL(SDL_GL_ResetAttributes());
	SDL(SDL_Quit());
}

void GL32Renderer::HandleSDL(SDL_Event &event) {
	Key key;
	Point2 mouseDelta;
	float controllerAxisValue;
	switch(event.type) {
	case SDL_QUIT:
		engine->Quit();
		break;
	case SDL_WINDOWEVENT:
		switch(event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			width = event.window.data1;
			height = event.window.data2;
			GL(glViewport(0, 0, width, height));
			DebugManager()->Trace("MakePipeline from SDL_WINDOWEVENT");
			MakePipeline(pipelineNames);
			DebugManager()->Trace("MakePipeline done");
			DebugManager()->Trace("SteamAPI_RunCallbacks before");
			SteamAPI_RunCallbacks();
			DebugManager()->Trace("SteamAPI_RunCallbacks after");
			break;
		}
		break;
	case SDL_KEYDOWN:
		if(event.key.repeat == 0) {
			key = mkKeyFromSDL(event.key.keysym.sym);
			engine->GetSystem<EventManager>().lock()->Fire("keydown", &key);
		}
		break;
	case SDL_KEYUP:
		key = mkKeyFromSDL(event.key.keysym.sym);
		engine->GetSystem<EventManager>().lock()->Fire("keyup", &key);
		break;
	case SDL_MOUSEBUTTONDOWN:
		key = mkMouseButtonFromSDL(event.button.button);
		engine->GetSystem<EventManager>().lock()->Fire("keydown", &key);
		break;
	case SDL_MOUSEBUTTONUP:
		key = mkMouseButtonFromSDL(event.button.button);
		engine->GetSystem<EventManager>().lock()->Fire("keyup", &key);
		break;
	case SDL_MOUSEMOTION:
		mouseDelta = Point2(event.motion.xrel, event.motion.yrel);
		engine->GetSystem<EventManager>().lock()->Fire("mousemove", &mouseDelta);
		break;
	case SDL_MOUSEWHEEL:
		mouseDelta = Point2(event.wheel.x, event.wheel.y);
		engine->GetSystem<EventManager>().lock()->Fire("mousewheel", &mouseDelta);
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		key = mkButtonFromSDL(static_cast<SDL_GameControllerButton>(event.cbutton.button));
		engine->GetSystem<EventManager>().lock()->Fire("keydown", &key);
		break;
	case SDL_CONTROLLERBUTTONUP:
		key = mkButtonFromSDL(static_cast<SDL_GameControllerButton>(event.cbutton.button));
		engine->GetSystem<EventManager>().lock()->Fire("keyup", &key);
		break;
	case SDL_CONTROLLERAXISMOTION:
		/* axis 0 = x axis
		 * axis 1 = y axis
		 */
		controllerAxisValue = event.caxis.value;
		switch(event.caxis.axis) {
		case 0:
			engine->GetSystem<EventManager>().lock()->Fire("controlleraxisx", Arg(controllerAxisValue));
			break;
		case 1:
			engine->GetSystem<EventManager>().lock()->Fire("controlleraxisy", Arg(controllerAxisValue));
			break;
		}
		break;
	}
}

void GL32Renderer::MakePipeline(const std::vector<std::string> &names) {
	pipelineNames = names;

	auto assetM = engine->GetSystem<AssetManager>().lock();
	std::vector<std::shared_ptr<ShaderProgramAsset>> assets;
	for(auto &node : nodes) {
		for(auto &out : node.globalOuts) { GL(glDeleteTextures(1, &out.second)); }
		for(auto &out : node.outs) { GL(glDeleteTextures(1, &out.second)); }
		GL(glDeleteFramebuffers(1, &node.fbo));
		GL(glDeleteProgram(node.program));
	}
	nodes.clear();

	for(auto &name : names) {
		DebugManager()->Verbose("building shader program `", name, '`');
		auto asset = assetM->Get<ShaderProgramAsset>(name);
		assets.emplace_back(asset);
		nodes.emplace_back(MakeProgram(asset));
		for(auto &uniform : asset->uniforms) { nodes.back().uniforms.emplace(uniform); }
	}

	for(unsigned int i = 0; i < nodes.size() - 1; ++i) {
		auto &asset = assets[i], &nextAsset = assets[i + 1];
		auto &node = nodes[i], &nextNode = nodes[i + 1];

		std::vector<GLenum> drawBuffers;
		int colorAttachment = 0;

		GL(glGenFramebuffers(1, &node.fbo));
		GL(glBindFramebuffer(GL_FRAMEBUFFER, node.fbo));

		auto fOut = [this, &drawBuffers, &colorAttachment] (ShaderProgramOutputAsset &out) {
			GLuint texture;
			GL(glGenTextures(1, &texture));
			GL(glBindTexture(GL_TEXTURE_2D, texture));

			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

			GLenum internalFormat = 0, format = 0, type = 0, attachment = 0;

			/* WARNING: 3-component sized internal formats are not required to be renderable */
			switch(out.type) {
			case ProgramOutputType::RGB8:
				internalFormat = GL_RGB8;
				format = GL_RGB;
				type = GL_UNSIGNED_BYTE;
				attachment = GL_COLOR_ATTACHMENT0;
				break;
			case ProgramOutputType::RGBA8:
				internalFormat = GL_RGBA8;
				format = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
				attachment = GL_COLOR_ATTACHMENT0;
				break;
			case ProgramOutputType::RGB16F:
				internalFormat = GL_RGB16F;
				format = GL_RGB;
				type = GL_HALF_FLOAT;
				attachment = GL_COLOR_ATTACHMENT0;
				break;
			case ProgramOutputType::RGBA16F:
				internalFormat = GL_RGBA16F;
				format = GL_RGBA;
				type = GL_HALF_FLOAT;
				attachment = GL_COLOR_ATTACHMENT0;
				break;
			case ProgramOutputType::RGB32F:
				internalFormat = GL_RGB32F;
				format = GL_RGB;
				type = GL_FLOAT;
				attachment = GL_COLOR_ATTACHMENT0;
				break;
			case ProgramOutputType::RGBA32F:
				internalFormat = GL_RGBA32F;
				format = GL_RGBA;
				type = GL_FLOAT;
				attachment = GL_COLOR_ATTACHMENT0;
				break;
			case ProgramOutputType::Depth:
				internalFormat = GL_DEPTH_COMPONENT24;
				format = GL_DEPTH_COMPONENT;
				type = GL_UNSIGNED_BYTE;
				attachment = GL_DEPTH_ATTACHMENT;
				break;
			case ProgramOutputType::Invalid:
				DebugManager()->Fatal("invalid program output type");
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

		for(auto &out : asset->outs) { node.outs.emplace(out.key, fOut(out)); }
		for(auto &out : asset->globalOuts) { node.globalOuts.emplace(out.key, fOut(out)); }

		for(auto &in : nextAsset->ins) {
			auto it = node.outs.find(in.key);
			if(it == node.outs.end()) { DebugManager()->Fatal("failed to connect nodes (invalid key `" + in.key + "`)"); }
			nextNode.ins.emplace(in.key, in.name);
		}

		for(auto &in : nextAsset->globalIns) { nextNode.globalIns.emplace(in.key, in.name); }

		GL(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));

		GLenum status;
		GL(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

		std::stringstream msg;
		switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			DebugManager()->Fatal("framebuffer unsupported");
		default:
			msg << "framebuffer status incomplete (0x" << std::hex << status << ')';
			DebugManager()->Fatal(msg.str());
		}
	}

	for(auto &node : nodes) {
		// upload projection matrix to pipeline stage
		Project(node.program);
		// upload resolution
		UploadUniform(node.program, "u_resolution", Point2(width, height));
	}

	for(auto uniform : uniformsU32) {
		SetUniform(uniform.first, uniform.second, true);
	}
	for(auto uniform : uniformsFloat) {
		SetUniform(uniform.first, uniform.second, true);
	}
	for(auto uniform : uniformsPoint3) {
		SetUniform(uniform.first, uniform.second, true);
	}
}

GLuint GL32Renderer::MakeProgram(std::shared_ptr<ShaderProgramAsset> asset) {
	std::vector<GLuint> ids;
	for(auto &shader : asset->shaders) {
		GLenum type = 0;
		switch(shader.type) {
		case ShaderType::Vertex:
			type = GL_VERTEX_SHADER;
			break;
		case ShaderType::Fragment:
			type = GL_FRAGMENT_SHADER;
			break;
		case ShaderType::Invalid:
			DebugManager()->Fatal("invalid shader type");
		}

		GLuint id;
		if(!GL(id = glCreateShader(type))) { DebugManager()->Fatal("failed to create shader"); }

		const GLchar *src = shader.source.c_str();
		const GLint len = static_cast<GLint>(shader.source.length());
		if(!GL(glShaderSource(id, 1, &src, &len))) { DebugManager()->Fatal("failed to upload shader source"); }
		if(!GL(glCompileShader(id))) { DebugManager()->Fatal("shader compilation is unsupported on this platform"); }

		GLint status;
		if(!GL(glGetShaderiv(id, GL_COMPILE_STATUS, &status))) { DebugManager()->Fatal("failed to get shader compilation status"); }

		if(status == GL_FALSE) {
			GLint infoLen;
			if(!GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLen))) { DebugManager()->Fatal("failed to get shader info log length"); }

			if(infoLen > 0) {
				char *infoLog = new char[infoLen];
				if(!GL(glGetShaderInfoLog(id, infoLen, NULL, infoLog))) { DebugManager()->Fatal("failed to get shader info log"); }
				DebugManager()->Debug("shader info log:");
				DebugManager()->Debug(infoLog);
				delete[] infoLog;
			}
			DebugManager()->Fatal("failed to compile shader");
		}

		ids.push_back(id);
	}

	GLuint programID;
	if(!GL(programID = glCreateProgram())) { DebugManager()->Fatal("failed to create program"); }

	for(auto id : ids) {
		if(!GL(glAttachShader(programID, id))) { DebugManager()->Fatal("failed to attach shader to program"); }

		/* flag shader for deletion */
		if(!GL(glDeleteShader(id))) { DebugManager()->Fatal("failed to flag shader for deletion"); }
	}

	if(!GL(glLinkProgram(programID))) { DebugManager()->Fatal("program linking is unsupported on this platform"); }

	GLint status;
	if(!GL(glGetProgramiv(programID, GL_LINK_STATUS, &status))) { DebugManager()->Fatal("failed to get program linking status"); }

	if(status == GL_FALSE) {
		GLint infoLen;
		if(!GL(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen))) { DebugManager()->Fatal("failed to get program info log length"); }

		if(infoLen > 0) {
			char *infoLog = new char[infoLen];
			if(!GL(glGetProgramInfoLog(programID, infoLen, NULL, infoLog))) { DebugManager()->Fatal("failed to get program info log"); }
			DebugManager()->Debug("program info log:");
			DebugManager()->Debug(infoLog);
			delete[] infoLog;
		}
		DebugManager()->Fatal("failed to link program");
	}
	return programID;
}
