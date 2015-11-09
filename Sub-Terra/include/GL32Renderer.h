#pragma once

#include <atomic>
#include <boost/array.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "sdl.h"
#include "gl.h"
#include "Renderer.h"
#include "ShaderProgramAsset.h"
#include "ModelComponent.h"

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
public:
	enum class MessageType {
		Add,
		Remove,
		Stop
	};

	struct Message {
		static Message Add(IDType id, boost::shared_ptr<ModelComponent> model) { return Message{MessageType::Add, id, model}; }
		static Message Remove(IDType id) { return Message{MessageType::Remove, id}; }
		static Message Stop() { return Message{MessageType::Stop}; }

		MessageType type;
		IDType id;
		boost::shared_ptr<ModelComponent> model;
	};
private:
	static const int channelSize = 8192;
	boost::array<Message, channelSize> channel;
	std::atomic_int channelWaiting = {0};
	int channelIndex = 0;

	bool threaded = false;
	std::thread thread;
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

	inline void ComponentAdded(IDType id, const std::type_info *ti, boost::weak_ptr<Component> ptr) override final {
		if(ti != &typeid(ModelComponent)) { return; }
		auto model = boost::static_pointer_cast<ModelComponent>(ptr.lock());

		channel[channelIndex] = Message::Add(id, model);
		++channelWaiting;
		++channelIndex;
		if(channelIndex == channelSize) { channelIndex = 0; }
	}

	inline void ComponentRemoved(IDType id, const std::type_info *ti) override final {
		if(ti != &typeid(ModelComponent)) { return; }
		auto model = engine->GetComponent<ModelComponent>(id);

		if(model != nullptr) {
			channel[channelIndex] = Message::Remove(id);
			++channelWaiting;
			++channelIndex;
			if(channelIndex == channelSize) { channelIndex = 0; }
		}
	}

	inline void Project(GLuint programID) {
		GL(glUseProgram(programID));

		GLint locProjection;
		GL(locProjection = glGetUniformLocation(programID, "u_projection"));
		if(locProjection == -1) { return; } /* -1 if uniform does not exist in program */

		auto heightF = static_cast<float>(height);
		auto fovy = 2.0f * glm::atan(heightF, 2.0f * pixelDistanceFromScreen) + fovPlus;
		glm::mat4 projection = glm::perspective(fovy, static_cast<float>(width) / heightF, zNear, zFar);
		//glm::mat4 projection = glm::infinitePerspective(fovy, static_cast<float>(width) / h, zNear);
		GL(glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection)));
	}
public:
	boost::unordered_map<std::string, float> uniforms;

	static bool IsSupported();
	GL32Renderer(Polar *engine, const std::vector<std::string> &names) : Renderer(engine), pipelineNames(names) {}
	~GL32Renderer();
	void MakePipeline(const std::vector<std::string> &) override final;

	inline void SetClearColor(const Point4 &color) override final {
		GL(glClearColor(color.x, color.y, color.z, color.w));
	}

	void SetUniform(const std::string &name, float x) {
		uniforms[name] = x;
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
