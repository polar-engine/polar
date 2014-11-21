#include "common.h"
#include "GL32Renderer.h"
#include "ModelComponent.h"
#include <iomanip>

bool GL32Renderer::IsSupported() {
	GL32Renderer renderer;
	try {
		auto e = std::runtime_error(BASEFILE);
		renderer.InitGL();
		GLint major, minor;
		if(!GL(glGetIntegerv(GL_MAJOR_VERSION, &major))) { throw e; }
		if(!GL(glGetIntegerv(GL_MINOR_VERSION, &minor))) { throw e; }
		/* if OpenGL version is 3.2 or greater */
		if(!(major > 3 || (major == 3 && minor >= 2))) {
			ENGINE_ERROR("actual OpenGL version is " << major << '.' << minor);
			throw e;
		}
		renderer.Destroy();
	} catch(std::exception &) {
		renderer.Destroy();
		return false;
	}
	return true;
}

void GL32Renderer::InitGL() {
	auto e = std::runtime_error(BASEFILE);
	if(!SDL(SDL_Init(SDL_INIT_EVERYTHING))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8))) { throw e; }
	if(!SDL(window = SDL_CreateWindow("Polar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN))) { throw e; }
	if(!SDL(context = SDL_GL_CreateContext(window))) { throw e; }
	if(!SDL(SDL_GL_SetSwapInterval(1))) { throw e; }

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if(err != GLEW_OK) {
		ENGINE_ERROR("GLEW: glewInit failed");
		throw e;
	}

	/* GLEW cals glGetString(EXTENSIONS) which
	 * causes GL_INVALID_ENUM on GL 3.2+ core contexts
	 */
	glGetError();
}

void GL32Renderer::Init() {
	InitGL();
	eventManager->ListenFor("renderer", "bgcolor", [] (Arg arg) {
		auto color = arg.Get<glm::fvec4>();
		GL(glClearColor(color->x, color->y, color->z, color->w));
	});
}

void GL32Renderer::Update(DeltaTicks &dt, std::vector<Object *> &objects) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		HandleSDL(event);
	}

	static glm::fvec4 color;
	color.x += 0.001f; color.y += 0.0025f; color.z += 0.005f;
	eventManager->FireIn("renderer", "bgcolor", &color);

	GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

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
	GL(glLoadIdentity());
	GL(glRotatef(rot * alpha + previousRot * (1 - alpha), 0, 0, 1));

	GL(glBegin(GL_TRIANGLES));
	for(auto object : objects) {
		auto model = object->GetComponent<ModelComponent>();
		if(model != nullptr) {
			for(auto point : model->points) {
				GL(glVertex4f(point.x, point.y, point.z, point.w));
			}
		}
	}
	IGNORE_GL(glEnd());

	SDL(SDL_GL_SwapWindow(window));
}

void GL32Renderer::Destroy() {
	SDL(SDL_GL_DeleteContext(context));
	SDL(SDL_DestroyWindow(window));
	SDL(SDL_GL_ResetAttributes());
	SDL(SDL_Quit());
}

void GL32Renderer::ObjectAdded(Object *object) {
	auto model = object->GetComponent<ModelComponent>();
	if(model != nullptr) {
		ENGINE_DEBUG("blah");
		auto &points = model->points;
		auto v = points.data();
		GLuint vao;
		GLuint vbo;
		GL(glGenVertexArrays(1, &vao));
		GL(glBindVertexArray(vao));
		GL(glGenBuffers(1, &vbo));
		GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * points.size(), points.data(), GL_STATIC_DRAW));
	}
}

void GL32Renderer::HandleSDL(SDL_Event &event) {
	switch(event.type) {
	case SDL_QUIT:
		eventManager->Fire("destroy");
		break;
	}
}
