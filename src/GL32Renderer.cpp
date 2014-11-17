#include "common.h"
#include "GL32Renderer.h"

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
	} catch(std::exception &e) {
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
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) { throw e; }
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
		auto color = arg.Get<glm::detail::fvec4SIMD>();
		GL(glClearColor(color->x, color->y, color->z, color->w));
	});

	auto color = glm::detail::fvec4SIMD(0.02f, 0.05f, 0.1f, 1.0f);
	eventManager->FireIn("renderer", "bgcolor", &color);
}

void GL32Renderer::Update(int) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		HandleSDL(event);
	}

	static glm::detail::fvec4SIMD color;
	color.x += 0.001f; color.y += 0.0025f; color.z += 0.005f;
	eventManager->FireIn("renderer", "bgcolor", &color);

	GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	SDL(SDL_GL_SwapWindow(window));
}

void GL32Renderer::Destroy() {
	SDL(SDL_GL_DeleteContext(context));
	SDL(SDL_DestroyWindow(window));
	SDL(SDL_GL_ResetAttributes());
	SDL(SDL_Quit());
}

void GL32Renderer::HandleSDL(SDL_Event &event) {
	switch(event.type) {
	case SDL_QUIT:
		eventManager->Fire("destroy");
		break;
	}
}