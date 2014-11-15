#include "stdafx.h"
#include "GL32Renderer.h"

bool GL32Renderer::IsSupported() {
	GL32Renderer renderer;
	try {
		renderer.Init();
		/* TODO: check OpenGL major/minor version */
		renderer.Destroy();
	} catch(std::exception &) {
		return false;
	}
	return true;
}

void GL32Renderer::Init() {
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
}

void GL32Renderer::Update(int) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		HandleSDL(event);
	}
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
