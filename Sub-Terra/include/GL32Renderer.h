#pragma once

#include <boost/array.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
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
	GLuint vao;
	std::vector<GLuint> vbos;
	GLsizei numVertices;
};

class GL32Renderer : public Renderer {
private:
	SDL_Window *window;
	SDL_GLContext context;
	boost::container::vector<std::string> pipelineNames;
	boost::container::vector<PipelineNode> nodes;
	boost::container::vector<boost::shared_ptr<GL32ModelProperty>> modelPropertyPool;

	GLuint viewportVAO;

	void Init() override final;
	void Update(DeltaTicks &) override final;

	inline boost::shared_ptr<GL32ModelProperty> GetPooledModelProperty() {
		if(modelPropertyPool.empty()) {
			GL32ModelProperty prop;

			GL(glGenVertexArrays(1, &prop.vao));
			GL(glBindVertexArray(prop.vao));

			/* location   attribute
			 *
			 *        0   vertex
			 *        1   normal
			 */

			prop.vbos.resize(2);
			GL(glGenBuffers(2, &prop.vbos[0]));

			GL(glBindBuffer(GL_ARRAY_BUFFER, prop.vbos[0]));
			GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));

			GL(glBindBuffer(GL_ARRAY_BUFFER, prop.vbos[1]));
			GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL));

			GL(glEnableVertexAttribArray(0));
			GL(glEnableVertexAttribArray(1));

			return boost::make_shared<GL32ModelProperty>(prop);
		} else {
			auto prop = modelPropertyPool.back();
			modelPropertyPool.pop_back();
			return prop;
		}
	}

	inline void ComponentAdded(IDType id, const std::type_info *ti, boost::weak_ptr<Component> ptr) override final {
		if(ti != &typeid(ModelComponent)) { return; }
		auto model = boost::static_pointer_cast<ModelComponent>(ptr.lock());
		auto prop = GetPooledModelProperty();
		auto normals = model->CalculateNormals();
		auto numVertices = normals.size();
		auto dataSize = sizeof(Point3) * numVertices;

		if(numVertices > prop->numVertices) {
			GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
			GL(glBufferData(GL_ARRAY_BUFFER, dataSize, model->points.data(), GL_DYNAMIC_DRAW));

			GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
			GL(glBufferData(GL_ARRAY_BUFFER, dataSize, normals.data(), GL_DYNAMIC_DRAW));
		} else {
			GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
			GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, model->points.data()));

			GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
			GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, normals.data()));
		}

		prop->numVertices = numVertices;

		model->Add<GL32ModelProperty>(prop);
		//model->points.clear();
		//model->points.shrink_to_fit();
	}

	inline void ComponentRemoved(IDType id, const std::type_info *ti) override final {
		if(ti != &typeid(ModelComponent)) { return; }
		auto model = engine->GetComponent<ModelComponent>(id);

		if(model != nullptr) {
			auto prop = model->Get<GL32ModelProperty>().lock();
			if(prop) {
				modelPropertyPool.emplace_back(prop);
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

	void InitGL();
	void HandleSDL(SDL_Event &);
	GLuint MakeProgram(ShaderProgramAsset &);
public:
	boost::unordered_map<std::string, float> uniforms;

	static bool IsSupported();
	GL32Renderer(Polar *engine, const boost::container::vector<std::string> &names) : Renderer(engine), pipelineNames(names) {}
	~GL32Renderer();
	void MakePipeline(const boost::container::vector<std::string> &) override final;

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
