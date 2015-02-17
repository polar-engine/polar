#pragma once

#include "Renderer.h"
#include "sdl.h"
#include "gl.h"
#include "ShaderAsset.h"

struct PipelineNode {
	GLuint program;
	GLuint fbo = 0;
	std::vector<GLuint> buffers;
	std::vector<std::string> bufferNames;
	PipelineNode(GLuint program) : program(program) {}
};

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
	SDL_GLContext context;
	GLuint activeProgram;
	std::vector<PipelineNode> nodes;

	void InitGL();
	void HandleSDL(SDL_Event &);
	GLuint MakeProgram(ShaderAsset &);
protected:
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
	void Destroy() override final;
	void ObjectAdded(Object *) override final;
	void ObjectRemoved(Object *) override final;
	void Project(GLuint);
public:
	static bool IsSupported();
	GL32Renderer(Polar *engine) : Renderer(engine) {}
	void SetClearColor(const Point4 &) override final;
	void Use(const std::string &) override final;
	void MakePipeline(const std::vector<std::string> &) override final;
};
