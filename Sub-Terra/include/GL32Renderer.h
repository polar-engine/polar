#pragma once

#include "sdl.h"
#include "gl.h"
#include "Renderer.h"
#include "ShaderProgramAsset.h"

struct PipelineNode {
	GLuint program;
	GLuint fbo = 0;
	std::unordered_map<std::string, GLuint> outs;
	std::unordered_map<std::string, std::string> ins;
	std::unordered_map<std::string, GLuint> globalOuts;
	std::unordered_map<std::string, std::string> globalIns;
	PipelineNode(GLuint program) : program(program) {}
};

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
	SDL_GLContext context;
	std::vector<std::string> pipelineNames;
	std::vector<PipelineNode> nodes;

	void InitGL();
	void HandleSDL(SDL_Event &);
	GLuint MakeProgram(ShaderProgramAsset &);
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
	void Destroy() override final;
	void ObjectAdded(Object *) override final;
	void ObjectRemoved(Object *) override final;
	void Project(GLuint);
public:
	static bool IsSupported();
	GL32Renderer(Polar *engine) : Renderer(engine) {}
	void SetClearColor(const Point4 &) override final;
	void MakePipeline(const std::vector<std::string> &) override final;
};
