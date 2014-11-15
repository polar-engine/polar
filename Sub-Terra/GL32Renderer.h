#pragma once

#include "Renderer.h"

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
	SDL_GLContext context;
	void HandleSDL(SDL_Event &);
public:
	static bool IsSupported();
	void Init() override final;
	void Update(int) override final;
	void Destroy() override final;
};
