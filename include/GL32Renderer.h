#pragma once

#include "Renderer.h"
#include "sdl.h"
#include "gl.h"

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
	SDL_GLContext context;
	void HandleSDL(SDL_Event &);
public:
	static bool IsSupported();
	void InitGL();
	void Init() override final;
	void Update(DeltaTicks, std::vector<Object *> &) override final;
	void Destroy() override final;
};
