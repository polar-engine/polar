#pragma once

#include "Renderer.h"
#include "sdl.h"
#include "gl.h"

class GL32Drawable {
public:
	size_t count;
	GLuint array;
	//std::unordered_map<const std::string, GLuint> buffers;
};

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
	SDL_GLContext context;
	void InitGL();
	void HandleSDL(SDL_Event &);
protected:
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
	void Destroy() override final;
	void ObjectAdded(Object *) override final;
public:
	static bool IsSupported();
	GL32Renderer(Polar *engine) : Renderer(engine) {}
	void SetClearColor(const glm::fvec4 &) override final;
};
