#include "stdafx.h"
#include "GL32Renderer.h"

bool GL32Renderer::IsSupported() {
	GL32Renderer renderer;
	try {
		auto e = std::runtime_error(BASEFILE);
		renderer.InitGL();
		GLint major, minor;
		if(!GL(glGetIntegerv(GL_MAJOR_VERSION, &major))) { throw e; }
		if(!GL(glGetIntegerv(GL_MINOR_VERSION, &minor))) { throw e; }
		if(major != 3 || minor != 2) { throw e; }
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
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))) { throw e; }
	if(!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8))) { throw e; }
	if(!SDL(window = SDL_CreateWindow("Polar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN))) { throw e; }
	if(!SDL(context = SDL_GL_CreateContext(window))) { throw e; }
	if(!SDL(SDL_GL_SetSwapInterval(1))) { throw e; }

	glewExperimental = GL_TRUE;

	GLenum err;
	GL(err = glewInit());

	if(err != GLEW_OK) {
		ERROR("GLEW: glewInit failed");
		throw e;
	}
}

void GL32Renderer::Init() {
	InitGL();
	eventManager->ListenFor("renderer", "bgcolor", [] (Arg arg) {
		auto &color = *arg.Get<Tuple4<float> *>();
		GL(glClearColor(std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color)));
	});
	Tuple4<float> color = std::make_tuple(0.02f, 0.05f, 0.1f, 0.0f);
	eventManager->FireIn("renderer", "bgcolor", &color);
}

void GL32Renderer::Update(int) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		HandleSDL(event);
	}

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
