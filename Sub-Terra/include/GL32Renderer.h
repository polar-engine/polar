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

class GL32ModelProperty : public Property {
public:
	const GLsizei numVertices;
	const GLuint vao;
	const std::vector<GLuint> vbos;
	GL32ModelProperty(const GLsizei numVertices, const GLuint vao, const std::vector<GLuint> &vbos) : numVertices(numVertices), vao(vao), vbos(vbos) {}
	GL32ModelProperty(const GLsizei numVertices, const GLuint vao, std::vector<GLuint> &&vbos) : numVertices(numVertices), vao(vao), vbos(vbos) {}
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

	inline void ComponentAdded(IDType id, const std::type_info *ti, boost::weak_ptr<Component> ptr) override final {
		if(ti != &typeid(ModelComponent)) { return; }
		auto model = boost::static_pointer_cast<ModelComponent>(ptr.lock());

		GLuint vao;
		GL(glGenVertexArrays(1, &vao));
		GL(glBindVertexArray(vao));

		/* location   attribute
		*
		*        0   vertex
		*        1   normal
		*/

		GLuint vbos[2];
		GL(glGenBuffers(2, vbos));

		GL(glBindBuffer(GL_ARRAY_BUFFER, vbos[0]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point3) * model->points.size(), model->points.data(), GL_STATIC_DRAW));
		GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));

		auto normals = model->CalculateNormals();
		GL(glBindBuffer(GL_ARRAY_BUFFER, vbos[1]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(Point3) * normals.size(), normals.data(), GL_STATIC_DRAW));
		GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL));

		GL(glEnableVertexAttribArray(0));
		GL(glEnableVertexAttribArray(1));

		std::vector<GLuint> vbosVector;
		for(unsigned int i = 0; i < sizeof(vbos) / sizeof(*vbos); ++i) {
			vbosVector.emplace_back(vbos[i]);
		}

		model->Add<GL32ModelProperty>(static_cast<GLsizei>(model->points.size()), vao, vbosVector);
		//model->points.clear();
		//model->points.shrink_to_fit();
	}

	inline void ComponentRemoved(IDType id, const std::type_info *ti) override final {
		if(ti != &typeid(ModelComponent)) { return; }
		auto model = engine->GetComponent<ModelComponent>(id);

		if(model != nullptr) {
			auto property = model->Get<GL32ModelProperty>().lock();
			if(property) {
				for(auto vbo : property->vbos) {
					GL(glDeleteBuffers(1, &vbo));
				}
				GL(glDeleteVertexArrays(1, &property->vao));
				model->Remove<GL32ModelProperty>();
			}
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
