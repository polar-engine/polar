#pragma once

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "sdl.h"
#include "gl.h"
#include "Renderer.h"
#include "ShaderProgramAsset.h"

struct PipelineNode {
	GLuint program;
	GLuint fbo = 0;
	boost::unordered_set<std::string> uniforms;
	boost::unordered_map<std::string, GLuint> outs;
	boost::unordered_map<std::string, std::string> ins;
	boost::unordered_map<std::string, GLuint> globalOuts;
	boost::unordered_map<std::string, std::string> globalIns;
	PipelineNode(GLuint program) : program(program) {}
};

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
	SDL_GLContext context;
	std::vector<std::string> pipelineNames;
	std::vector<PipelineNode> nodes;
	GLuint viewportVAO;

	void InitGL();
	void HandleSDL(SDL_Event &);
	GLuint MakeProgram(ShaderProgramAsset &);
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
	void ComponentAdded(IDType, const std::type_info *, std::weak_ptr<Component>) override final;
	void ComponentRemoved(IDType, const std::type_info *) override final;
	void Project(GLuint);
public:
	static bool IsSupported();
	GL32Renderer(Polar *engine, const std::vector<std::string> &names) : Renderer(engine), pipelineNames(names) {}
	~GL32Renderer();

	void SetClearColor(const Point4 &) override final;
	void MakePipeline(const std::vector<std::string> &) override final;

	void SetUniform(const std::string &name, float x) {
		for(auto &node : nodes) {
			if(node.uniforms.find(name) != node.uniforms.end()) {
				GL(glUseProgram(node.program));
				GLint loc;
				GL(loc = glGetUniformLocation(node.program, name.c_str()));
				GL(glUniform1f(loc, x));
			}
		}
	}
};
