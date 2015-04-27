#include "common.h"
#include "GL32Renderer.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "Integrator.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "ModelComponent.h"
#include "PlayerCameraComponent.h"

class GL32ModelProperty : public Property {
public:
	const GLsizei numVertices;
	const GLuint vao;
	const std::vector<GLuint> vbos;
	GL32ModelProperty(const GLsizei numVertices, const GLuint vao, const std::vector<GLuint> &vbos) : numVertices(numVertices), vao(vao), vbos(vbos) {}
	GL32ModelProperty(const GLsizei numVertices, const GLuint vao, std::vector<GLuint> &&vbos) : numVertices(numVertices), vao(vao), vbos(vbos) {}
};

bool GL32Renderer::IsSupported() {
	GL32Renderer renderer(nullptr);
	try {
		renderer.InitGL();
		GLint major, minor;
		if(!GL(glGetIntegerv(GL_MAJOR_VERSION, &major))) { ENGINE_THROW("failed to get OpenGL major version"); }
		if(!GL(glGetIntegerv(GL_MINOR_VERSION, &minor))) { ENGINE_THROW("failed to get OpenGL minor version"); }
		/* if OpenGL version is 3.2 or greater */
 		if(!(major > 3 || (major == 3 && minor >= 2))) {
			std::stringstream msg;
			msg << "actual OpenGL version is " << major << '.' << minor;
			ENGINE_THROW(msg.str());
		}
		return true;
	} catch(std::exception &) {
		return false;
	}
}

void GL32Renderer::InitGL() {
	if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) { ENGINE_THROW("failed to init SDL"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) { ENGINE_THROW("failed to set major version attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2))) { ENGINE_THROW("failed to set minor version attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) { ENGINE_THROW("failed to set profile mask attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) { ENGINE_THROW("failed to set double buffer attribute"); }
	if(!SDL(window = SDL_CreateWindow("Polar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE))) {
		ENGINE_THROW("failed to create window");
	}
	if(!SDL(context = SDL_GL_CreateContext(window))) { ENGINE_THROW("failed to create OpenGL context"); }
	if(!SDL(SDL_GL_SetSwapInterval(1))) { ENGINE_THROW("failed to set swap interval"); }
	if(!SDL(SDL_SetRelativeMouseMode(SDL_TRUE))) { ENGINE_THROW("failed to set relative mouse mode"); }

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if(err != GLEW_OK) { ENGINE_THROW("GLEW: glewInit failed"); }

	/* GLEW cals glGetString(EXTENSIONS) which
	 * causes GL_INVALID_ENUM on GL 3.2+ core contexts
	 */
	glGetError();

	GL(glEnable(GL_DEPTH_TEST));
	GL(glEnable(GL_BLEND));
	GL(glEnable(GL_CULL_FACE));
	GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL(glCullFace(GL_BACK));
}

void GL32Renderer::Init() {
	InitGL();
	SetClearColor(Point4(0.02f, 0.05f, 0.1f, 1));
	//ENGINE_OUTPUT(engine->systems.Get<AssetManager>()->Get<TextAsset>("hello").text << '\n');

	GL(glGenVertexArrays(1, &viewportVAO));
	GL(glBindVertexArray(viewportVAO));

	GLuint vbo;
	GL(glGenBuffers(1, &vbo));

	std::vector<Point2> points = {
		Point2(-1, -1),
		Point2( 1, -1),
		Point2(-1,  1),
		Point2( 1,  1)
	};

	GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point2) * 4, points.data(), GL_STATIC_DRAW));
	GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL));

	GL(glEnableVertexAttribArray(0));
}

void GL32Renderer::Update(DeltaTicks &dt) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		HandleSDL(event);
	}
	SDL_ClearError();

	auto integrator = engine->systems.Get<Integrator>().lock();
	float alpha = integrator->Accumulator().Seconds();

	glm::mat4 cameraView;
	auto pairRight = engine->objects.right.equal_range(&typeid(PlayerCameraComponent));
	for(auto itRight = pairRight.first; itRight != pairRight.second; ++itRight) {
		auto camera = static_cast<PlayerCameraComponent *>(itRight->info.get());

		PositionComponent *pos = nullptr;
		OrientationComponent *orient = nullptr;

		auto pairLeft = engine->objects.left.equal_range(itRight->get_left());
		for(auto itLeft = pairLeft.first; itLeft != pairLeft.second; ++itLeft) {
			auto type = itLeft->get_right();
			if(type == &typeid(PositionComponent)) { pos = static_cast<PositionComponent *>(itLeft->info.get()); }
			else if(type == &typeid(OrientationComponent)) { orient = static_cast<OrientationComponent *>(itLeft->info.get()); }
		}

		cameraView = glm::translate(cameraView, -camera->distance.Temporal(alpha).To<glm::vec3>());
		cameraView *= glm::toMat4(camera->orientation);

		if(orient != nullptr) {
			cameraView *= glm::toMat4(orient->orientation);
		}

		cameraView = glm::translate(cameraView, -camera->position.Temporal(alpha).To<glm::vec3>());

		if(pos != nullptr) {
			cameraView = glm::translate(cameraView, -pos->position.Temporal(alpha).To<glm::vec3>());
		}
	}

	std::unordered_map<std::string, GLuint> globals;
	for(unsigned int i = 0; i < nodes.size(); ++i) {
		auto &node = nodes[i];

		GL(glBindFramebuffer(GL_FRAMEBUFFER, node.fbo));
		GL(glUseProgram(node.program));
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		switch(i) {
		case 0: {
			GLint locView;
			GL(locView = glGetUniformLocation(node.program, "u_view"));
			GL(glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(cameraView)));
			GLint locModel;
			GL(locModel = glGetUniformLocation(node.program, "u_model"));

			auto pairRight = engine->objects.right.equal_range(&typeid(ModelComponent));
			for(auto itRight = pairRight.first; itRight != pairRight.second; ++itRight) {
				auto model = static_cast<ModelComponent *>(itRight->info.get());
				glm::mat4 modelMatrix;

				PositionComponent *pos = nullptr;
				OrientationComponent *orient = nullptr;

				auto pairLeft = engine->objects.left.equal_range(itRight->get_left());
				for(auto itLeft = pairLeft.first; itLeft != pairLeft.second; ++itLeft) {
					auto type = itLeft->get_right();
					if(type == &typeid(PositionComponent)) { pos = static_cast<PositionComponent *>(itLeft->info.get()); }
					else if(type == &typeid(OrientationComponent)) { orient = static_cast<OrientationComponent *>(itLeft->info.get()); }
				}

				auto property = model->Get<GL32ModelProperty>().lock();
				if(property) {
					glm::mat4 modelView = cameraView;

					if(pos != nullptr) { modelMatrix = glm::translate(modelMatrix, pos->position.Temporal(alpha).To<glm::vec3>()); }
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

					GL(glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelMatrix)));
					GL(glBindVertexArray(property->vao));
					GL(glDrawArrays(drawMode, 0, property->numVertices));
				}
			}
			break;
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

				GLint locBuffer;
				GL(locBuffer = glGetUniformLocation(node.program, pair.second.c_str()));
				GL(glUniform1i(locBuffer, b));
				++b;
			}
			for(auto &pair : node.globalIns) { /* for each global input*/
				auto buffer = globals[pair.first];
				GL(glActiveTexture(GL_TEXTURE0 + b));
				GL(glBindTexture(GL_TEXTURE_2D, buffer));

				GLint locBuffer;
				GL(locBuffer = glGetUniformLocation(node.program, pair.second.c_str()));
				GL(glUniform1i(locBuffer, b));
				++b;
			}

			GL(glBindVertexArray(viewportVAO));
			GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

			break;
		}
	}

	SDL(SDL_GL_SwapWindow(window));
}

GL32Renderer::~GL32Renderer() {
	SDL(SDL_GL_DeleteContext(context));
	SDL(SDL_DestroyWindow(window));
	SDL(SDL_GL_ResetAttributes());
	SDL(SDL_Quit());
}

void GL32Renderer::ComponentAdded(IDType id, const std::type_info *ti, std::weak_ptr<Component> ptr) {
	if(ti != &typeid(ModelComponent)) { return; }
	auto model = static_cast<ModelComponent *>(ptr.lock().get());

	GLuint vao;
	GL(glGenVertexArrays(1, &vao));
	GL(glBindVertexArray(vao));

	/* location   attribute
	*
	*        0   vertex
	*        1   normal
	*/

	GLuint vbos[2];
	GL(glGenBuffers(2, vbos));

	GL(glBindBuffer(GL_ARRAY_BUFFER, vbos[0]));
	GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point3) * model->points.size(), model->points.data(), GL_STATIC_DRAW));
	GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));

	auto normals = model->CalculateNormals();
	GL(glBindBuffer(GL_ARRAY_BUFFER, vbos[1]));
	GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point3) * normals.size(), normals.data(), GL_STATIC_DRAW));
	GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL));

	GL(glEnableVertexAttribArray(0));
	GL(glEnableVertexAttribArray(1));

	std::vector<GLuint> vbosVector;
	for(unsigned int i = 0; i < sizeof(vbos) / sizeof(*vbos); ++i) {
		vbosVector.emplace_back(vbos[i]);
	}

	model->Add<GL32ModelProperty>(static_cast<GLsizei>(model->points.size()), vao, vbosVector);
	model->points.clear();
	model->points.shrink_to_fit();
}

void GL32Renderer::ComponentRemoved(IDType id, const std::type_info *ti) {
	if(ti != &typeid(ModelComponent)) { return; }
	auto model = engine->GetComponent<ModelComponent>(id);

	if(model != nullptr) {
		auto property = model->Get<GL32ModelProperty>().lock();
		if(property) {
			for(auto vbo : property->vbos) {
				GL(glDeleteBuffers(1, &vbo));
			}
			GL(glDeleteVertexArrays(1, &property->vao));
			model->Remove<GL32ModelProperty>();
		}
	}
}

void GL32Renderer::HandleSDL(SDL_Event &event) {
	Key key;
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
			MakePipeline(pipelineNames); /* TODO: XXX: really bad, not cleaning up OpenGL objects */
			break;
		}
		break;
	case SDL_KEYDOWN:
		if(event.key.repeat == 0) {
			key = mkKeyFromSDL(event.key.keysym.sym);
			engine->systems.Get<EventManager>().lock()->Fire("keydown", &key);
		}
		break;
	case SDL_KEYUP:
		key = mkKeyFromSDL(event.key.keysym.sym);
		engine->systems.Get<EventManager>().lock()->Fire("keyup", &key);
		break;
	case SDL_MOUSEMOTION:
		Point2 delta(event.motion.xrel, event.motion.yrel);
		engine->systems.Get<EventManager>().lock()->Fire("mousemove", &delta);
		break;
	}
}

void GL32Renderer::Project(GLuint programID) {
	GL(glUseProgram(programID));

	GLint locProjection;
	GL(locProjection = glGetUniformLocation(programID, "u_projection"));
	if(locProjection == -1) { return; } /* -1 if uniform does not exist in program */

	//glm::mat4 projection = glm::infinitePerspective(fovy, static_cast<float>(width) / static_cast<float>(height), zNear);
	glm::mat4 projection = glm::perspective(glm::radians(fovy), static_cast<float>(width) / static_cast<float>(height), zNear, zFar);
	GL(glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection)));
}

void GL32Renderer::SetClearColor(const Point4 &color) {
	GL(glClearColor(color.x, color.y, color.z, color.w));
}

void GL32Renderer::MakePipeline(const std::vector<std::string> &names) {
	pipelineNames = names;

	std::vector<ShaderProgramAsset> assets;
	nodes.clear();

	for(auto &name : names) {
		INFOS("loading shader asset `" << name << '`');
		auto asset = engine->systems.Get<AssetManager>().lock()->Get<ShaderProgramAsset>(name);
		assets.emplace_back(asset);
		nodes.emplace_back(MakeProgram(asset));
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

			GLenum internalFormat, format, type, attachment;

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
				ENGINE_THROW("invalid program output type");
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

		for(auto &out : asset.outs.elements) { node.outs.emplace(out.key.text, fOut(out)); }
		for(auto &out : asset.globalOuts.elements) { node.globalOuts.emplace(out.key.text, fOut(out)); }

		for(auto &in : nextAsset.ins.elements) {
			auto it = node.outs.find(in.key.text);
			if(it == node.outs.end()) { ENGINE_THROW("failed to connect nodes (invalid key `" + in.key.text + "`)"); }
			nextNode.ins.emplace(in.key.text, in.name.text);
		}

		for(auto &in : nextAsset.globalIns.elements) { nextNode.globalIns.emplace(in.key.text, in.name.text); }

		GL(glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data()));

		GLenum status;
		GL(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

		std::stringstream msg;
		switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			ENGINE_THROW("framebuffer unsupported");
		default:
			msg << "framebuffer status incomplete (0x" << std::hex << status << ')';
			ENGINE_THROW(msg.str());
		}

		/* upload projection matrix to pipeline stage */
		Project(node.program);
	}
}

GLuint GL32Renderer::MakeProgram(ShaderProgramAsset &asset) {
	std::vector<GLuint> ids;
	for(auto &shader : asset.shaders.elements) {
		GLenum type = 0;
		switch(shader.type) {
		case ShaderType::Vertex:
			type = GL_VERTEX_SHADER;
			break;
		case ShaderType::Fragment:
			type = GL_FRAGMENT_SHADER;
			break;
		case ShaderType::Invalid:
			ENGINE_THROW("invalid shader type");
		}

		GLuint id;
		if(!GL(id = glCreateShader(type))) { ENGINE_THROW("failed to create shader"); }

		const GLchar *src = shader.source.text.c_str();
		const GLint len = static_cast<GLint>(shader.source.text.length());
		if(!GL(glShaderSource(id, 1, &src, &len))) { ENGINE_THROW("failed to upload shader source"); }
		if(!GL(glCompileShader(id))) { ENGINE_THROW("shader compilation is unsupported on this platform"); }

		GLint status;
		if(!GL(glGetShaderiv(id, GL_COMPILE_STATUS, &status))) { ENGINE_THROW("failed to get shader compilation status"); }

		if(status == GL_FALSE) {
			GLint infoLen;
			if(!GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLen))) { ENGINE_THROW("failed to get shader info log length"); }

			if(infoLen > 0) {
				char *infoLog = new char[infoLen];
				if(!GL(glGetShaderInfoLog(id, infoLen, NULL, infoLog))) { ENGINE_THROW("failed to get shader info log"); }
				ENGINE_ERROR(infoLog);
				delete[] infoLog;
			}
			ENGINE_THROW("failed to compile shader");
		}

		ids.push_back(id);
	}

	GLuint programID;
	if(!GL(programID = glCreateProgram())) { ENGINE_THROW("failed to create program"); }

	for(auto id : ids) {
		if(!GL(glAttachShader(programID, id))) { ENGINE_THROW("failed to attach shader to program"); }

		/* flag shader for deletion */
		if(!GL(glDeleteShader(id))) { ENGINE_THROW("failed to flag shader for deletion"); }
	}

	if(!GL(glLinkProgram(programID))) { ENGINE_THROW("program linking is unsupported on this platform"); }

	GLint status;
	if(!GL(glGetProgramiv(programID, GL_LINK_STATUS, &status))) { ENGINE_THROW("failed to get program linking status"); }

	if(status == GL_FALSE) {
		GLint infoLen;
		if(!GL(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen))) { ENGINE_THROW("failed to get program info log length"); }

		if(infoLen > 0) {
			char *infoLog = new char[infoLen];
			if(!GL(glGetProgramInfoLog(programID, infoLen, NULL, infoLog))) { ENGINE_THROW("failed to get program info log"); }
			ENGINE_ERROR(infoLog);
			delete[] infoLog;
		}
		ENGINE_THROW("failed to link program");
	}
	return programID;
}
