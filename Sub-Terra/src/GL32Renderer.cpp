#include "common.h"
#include "GL32Renderer.h"
#include "PositionComponent.h"
#include "OrientationComponent.h"
#include "ModelComponent.h"
#include "PlayerCameraComponent.h"

class GL32ModelProperty : public Property {
public:
	const GLuint vao;
	GL32ModelProperty(const GLuint vao) : vao(vao) {}
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
		renderer.Destroy();
	} catch(std::exception &) {
		renderer.Destroy();
		return false;
	}
	return true;
}

void GL32Renderer::InitGL() {
	if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) { ENGINE_THROW("failed to init SDL"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) { ENGINE_THROW("failed to set major version attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2))) { ENGINE_THROW("failed to set minor version attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) { ENGINE_THROW("failed to set profile mask attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) { ENGINE_THROW("failed to set double buffer attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))) { ENGINE_THROW("failed to set multisample buffers attribute"); }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8))) { ENGINE_THROW("failed to set multisample samples attribute"); }
	if(!SDL(window = SDL_CreateWindow("Polar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE))) { ENGINE_THROW("failed to create window"); }
	if(!SDL(context = SDL_GL_CreateContext(window))) { ENGINE_THROW("failed to create OpenGL context"); }
	if(!SDL(SDL_GL_SetSwapInterval(1))) { ENGINE_THROW("failed to set swap interval"); }

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
	GL(glEnable(GL_MULTISAMPLE));
	GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL(glCullFace(GL_BACK));
}

void GL32Renderer::Init() {
	width = 1280;
	height = 720;
	fovy = 70;
	zNear = 0.05f;
	InitGL();
	SetClearColor(Point(0.02f, 0.05f, 0.1f, 1));
	ENGINE_OUTPUT(engine->systems.Get<AssetManager>()->Get<TextAsset>("hello").text << '\n');
}

void GL32Renderer::Update(DeltaTicks &dt, std::vector<Object *> &objects) {
	auto seconds = dt.count() / static_cast<float>(ENGINE_TICKS_PER_SECOND);

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		HandleSDL(event);
	}
	SDL_ClearError();

	GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	/*
	static float rot = 0;
	static float previousRot = 0;

	static DeltaTicks accumulator;
	const DeltaTicks timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / 7);
	accumulator += dt;

	while(accumulator >= timestep) {
		previousRot = rot;
		rot += 90.0f * timestep.count() / ENGINE_TICKS_PER_SECOND;
		accumulator -= timestep;
	}

	float alpha = (float)accumulator.count() / (float)timestep.count();
	float interpRot = rot * alpha + previousRot * (1 - alpha);
	auto qRot = glm::quat(glm::vec3(0, 0, interpRot * 3.1415 / 180));
	*/

	GLint locModelView;
	GL(locModelView = glGetUniformLocation(activeProgram, "u_modelView"));

	glm::mat4 cameraView;
	for(auto object : objects) {
		auto camera = object->Get<PlayerCameraComponent>();
		if(camera != nullptr) {
			cameraView = glm::translate(cameraView, glm::vec3(-camera->distance));
			cameraView *= glm::toMat4(camera->orientation);

			auto pos = object->Get<PositionComponent>();
			if(pos != nullptr) {
				cameraView = glm::translate(cameraView, glm::vec3(-pos->position));
			}
			cameraView = glm::translate(cameraView, glm::vec3(-camera->position));

			camera->orientation *= glm::quat(glm::vec3(0, 0.5f * seconds, 0));
		}
	}

	for(auto object : objects) {
		auto model = object->Get<ModelComponent>();
		if(model != nullptr) {
			auto property = model->Get<GL32ModelProperty>();
			if(property != nullptr) {
				glm::mat4 modelView = cameraView;

				auto pos = object->Get<PositionComponent>();
				if(pos != nullptr) {
					modelView = glm::translate(modelView, glm::fvec3(pos->position));
				}

				auto orient = object->Get<OrientationComponent>();
				if(orient != nullptr) {
					modelView *= glm::toMat4(orient->orientation);
					//orient->orientation *= glm::quat(glm::vec3(0, seconds * 70 * 3.1415 / 180, 0));
					orient->orientation = glm::quat(glm::vec3(seconds * 21 * 3.1415 / 180, 0, 0)) * orient->orientation * glm::quat(glm::vec3(0, seconds * 70 * 3.1415 / 180, 0));
				}

				GLenum drawMode = GL_TRIANGLES;
				switch(model->type) {
				case GeometryType::Triangles:
					drawMode = GL_TRIANGLES;
					break;
				case GeometryType::TriangleStrip:
					drawMode = GL_TRIANGLE_STRIP;
					break;
				}

				GL(glUniformMatrix4fv(locModelView, 1, GL_FALSE, glm::value_ptr(modelView)));
				GL(glBindVertexArray(property->vao));
				GL(glDrawArrays(drawMode, 0, static_cast<GLsizei>(model->points.size())));
			}
		}
	}

	SDL(SDL_GL_SwapWindow(window));
}

void GL32Renderer::Destroy() {
	SDL(SDL_GL_DeleteContext(context));
	SDL(SDL_DestroyWindow(window));
	SDL(SDL_GL_ResetAttributes());
	SDL(SDL_Quit());
}

void GL32Renderer::ObjectAdded(Object *object) {
	auto model = object->Get<ModelComponent>();
	if(model != nullptr) {
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
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * model->points.size(), model->points.data(), GL_STATIC_DRAW));
		GL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL));

		GL(glBindBuffer(GL_ARRAY_BUFFER, vbos[1]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * model->normals.size(), model->normals.data(), GL_STATIC_DRAW));
		GL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL));

		GL(glEnableVertexAttribArray(0));
		GL(glEnableVertexAttribArray(1));

		model->Add<GL32ModelProperty>(vao);
	}
}

void GL32Renderer::HandleSDL(SDL_Event &event) {
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
			Project();
			break;
		}
		break;
	case SDL_KEYDOWN:
		switch(event.key.keysym.sym) {
		case SDLK_w:
			break;
		}
		break;
	}
}

void GL32Renderer::Project() {
	GLint locProjection;
	GL(locProjection = glGetUniformLocation(activeProgram, "u_projection"));
	glm::mat4 projection = glm::infinitePerspective(fovy, static_cast<float>(width) / static_cast<float>(height), zNear);
	GL(glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection)));
}

void GL32Renderer::SetClearColor(const Point &color) {
	GL(glClearColor(color.x, color.y, color.z, color.w));
}

void GL32Renderer::Use(const std::string &name) {
	auto asset = engine->systems.Get<AssetManager>()->Get<ShaderAsset>(name);
	std::vector<GLuint> ids;
	for(auto &shader : asset.shaders) {
		GLenum type = 0;
		switch(shader.type) {
		case ShaderType::Vertex:
			type = GL_VERTEX_SHADER;
			break;
		case ShaderType::Fragment:
			type = GL_FRAGMENT_SHADER;
			break;
		default:
			ENGINE_THROW("invalid shader type");
		}

		GLuint id;
		if(!GL(id = glCreateShader(type))) { ENGINE_THROW("failed to create shader"); }

		const GLchar *src = shader.source.c_str();
		const GLint len = static_cast<GLint>(shader.source.length());
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

	if(!GL(glUseProgram(programID))) { ENGINE_THROW("failed to use program"); }

	activeProgram = programID;

	/* set projection matrix in new program */
	Project();
}
