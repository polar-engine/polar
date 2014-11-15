#include "stdafx.h"
#include "GL32Renderer.h"

bool GL32Renderer::IsSupported() {
	SDL_Window *window;
	SDL_GLContext context;
	if (!SDL(SDL_Init(SDL_INIT_EVERYTHING))) { return false; }
	if (!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))) { return false; }
	if (!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2))) { return false; }
	if (!SDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))) { return false; }
	if (!SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))) { return false; }
	if (!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))) { return false; }
	if (!SDL(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8))) { return false; }
	if (!SDL(window = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN))) { return false; }
	if (!SDL(context = SDL_GL_CreateContext(window))) { return false; }
	if (!SDL(SDL_GL_SetSwapInterval(1))) { return false; }
	/* TODO: check OpenGL major/minor version */
	SDL(SDL_GL_DeleteContext(context));
	SDL(SDL_DestroyWindow(window));
	SDL(SDL_GL_ResetAttributes());
	SDL(SDL_Quit());
	return true;
}

void GL32Renderer::Init() {

}

void GL32Renderer::Update(int) {

}

void GL32Renderer::Destroy() {

}
