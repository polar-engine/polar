#pragma once

#include "Renderer.h"

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
public:
	static bool IsSupported();
	void Init() override final;
	void Update(int) override final;
	void Destroy() override final;
};
