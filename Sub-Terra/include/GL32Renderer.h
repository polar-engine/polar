#pragma once

#include <functional>
#include <boost/array.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "sdl.h"
#include "gl.h"
#include "Renderer.h"
#include "ShaderProgramAsset.h"
#include "ModelComponent.h"
#include "Sprite.h"

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

struct GL32ModelProperty : public Property {
	GLuint vao;
	std::vector<GLuint> vbos;
	GLsizei numVertices = 0;
	GLsizei capacity = 0;

	inline friend bool operator<(const GL32ModelProperty &left, const GL32ModelProperty &right) {
		return left.capacity < right.capacity;
	}
};

struct GL32SpriteProperty : public Property {
	GLuint texture;
};

template<typename T> struct SharedPtrLess : public std::binary_function<boost::shared_ptr<T>, boost::shared_ptr<T>, bool> {
	inline bool operator()(const boost::shared_ptr<T> &left, const boost::shared_ptr<T> &right) const {
		return *left < *right;
	}
};

class GL32Renderer : public Renderer {
private:
	bool inited = false;

	SDL_Window *window;
	SDL_GLContext context;
	boost::container::vector<std::string> pipelineNames;
	boost::container::vector<PipelineNode> nodes;
	boost::container::flat_multiset<boost::shared_ptr<GL32ModelProperty>, SharedPtrLess<GL32ModelProperty>> modelPropertyPool;

	GLuint viewportVAO;
	GLuint spriteProgram;

	boost::shared_ptr<Destructor> fpsDtor;
	IDType fpsID = 0;

	boost::unordered_map<std::string, glm::uint32> uniformsU32;
	boost::unordered_map<std::string, Decimal> uniformsFloat;
	boost::unordered_map<std::string, Point3> uniformsPoint3;

	void Init() override final;
	void Update(DeltaTicks &) override final;

	inline boost::shared_ptr<GL32ModelProperty> GetPooledModelProperty(const GLsizei required) {
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
			auto dummy = boost::make_shared<GL32ModelProperty>();
			dummy->capacity = required;

			auto it = modelPropertyPool.lower_bound(dummy);
			if(it == modelPropertyPool.cend()) { it = modelPropertyPool.begin(); }

			auto prop = *it;
			modelPropertyPool.erase(it);
			return prop;
		}
	}

	inline void UploadModel(boost::shared_ptr<ModelComponent> model) {
		auto normals = model->CalculateNormals();
		auto numVertices = normals.size();
		auto dataSize = sizeof(ModelComponent::PointType) * numVertices;
		auto prop = GetPooledModelProperty(numVertices);

		if(numVertices > 0) {
			if(GLsizei(numVertices) > prop->capacity) {
				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
				GL(glBufferData(GL_ARRAY_BUFFER, dataSize, model->points.data(), GL_DYNAMIC_DRAW));

				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
				GL(glBufferData(GL_ARRAY_BUFFER, dataSize, normals.data(), GL_DYNAMIC_DRAW));

				prop->capacity = numVertices;
			} else {
				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[0]));
				GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, model->points.data()));

				GL(glBindBuffer(GL_ARRAY_BUFFER, prop->vbos[1]));
				GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, normals.data()));
			}
		}

		prop->numVertices = numVertices;

		model->Add<GL32ModelProperty>(prop);
		//model->points.clear();
		//model->points.shrink_to_fit();
	}

	inline void ComponentAdded(IDType id, const std::type_info *ti, boost::weak_ptr<Component> ptr) override final {
		if(ti == &typeid(ModelComponent)) {
			auto model = boost::static_pointer_cast<ModelComponent>(ptr.lock());
			UploadModel(model);
		} else if(ti == &typeid(Sprite)) {
			auto sprite = boost::static_pointer_cast<Sprite>(ptr.lock());
			sprite->Render();
			GL32SpriteProperty prop;

			GL(glGenTextures(1, &prop.texture));
			GL(glBindTexture(GL_TEXTURE_2D, prop.texture));

			GLint format = GL_RGBA;
			GL(glTexImage2D(GL_TEXTURE_2D, 0, format, sprite->surface->w, sprite->surface->h, 0, format, GL_UNSIGNED_BYTE, sprite->surface->pixels));
			GL(glGenerateMipmap(GL_TEXTURE_2D));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));

			sprite->Add<GL32SpriteProperty>(prop);
		}
	}

	inline void ComponentRemoved(IDType id, const std::type_info *ti) override final {
		if(ti == &typeid(ModelComponent)) {
			auto model = engine->GetComponent<ModelComponent>(id);
			if(model != nullptr) {
				auto prop = model->Get<GL32ModelProperty>().lock();
				if(prop) {
					modelPropertyPool.emplace(prop);
				}
			}
		} else if(ti == &typeid(Sprite)) {
			auto text = engine->GetComponent<Sprite>(id);
			if(text != nullptr) {
				auto prop = text->Get<GL32SpriteProperty>().lock();
				if(prop) {
					GL(glDeleteTextures(1, &prop->texture));
				}
			}
		}
	}

	inline Mat4 CalculateProjection() {
		auto heightF = static_cast<Decimal>(height);
		auto fovy = 2.0f * glm::atan(heightF, Decimal(2) * pixelDistanceFromScreen) + fovPlus;
		auto projection = glm::perspective(fovy, static_cast<Decimal>(width) / heightF, zNear, zFar);
		//auto projection = glm::infinitePerspective(fovy, static_cast<Decimal>(width) / heightF, zNear);
		return projection;
	}

	inline void Project(GLuint programID) {
		GL(glUseProgram(programID));
		UploadUniform(programID, "u_projection", CalculateProjection());
	}

	void InitGL();
	void HandleSDL(SDL_Event &);
	void MakePipeline(const boost::container::vector<std::string> &) override final;
	GLuint MakeProgram(std::shared_ptr<ShaderProgramAsset>);
public:
	Decimal fps = 60.0;

	static bool IsSupported();
	GL32Renderer(Polar *engine, const boost::container::vector<std::string> &names) : Renderer(engine), pipelineNames(names) {}
	~GL32Renderer();

	void SetPipeline(const boost::container::vector<std::string> &names) override final {
		pipelineNames = names;
		if(inited) {
			DebugManager()->Trace("MakePipeline from SetPipeline");
			MakePipeline(names);
			DebugManager()->Trace("MakePipeline done");
		}
	}

	inline void SetClearColor(const Point4 &color) override final {
		auto color2 = glm::vec4(color);
		GL(glClearColor(color.r, color.g, color.b, color.a));
	}

	Decimal GetUniformDecimal(const std::string &name, const Decimal def) override final {
		auto it = uniformsFloat.find(name);
		if(it != uniformsFloat.end()) {
			return it->second;
		} else {
			SetUniform(name, def);
			return def;
		}
	}

	Point3 GetUniformPoint3(const std::string &name, const Point3 def) override final {
		auto it = uniformsPoint3.find(name);
		if(it != uniformsPoint3.end()) {
			return it->second;
		} else {
			SetUniform(name, def);
			return def;
		}
	}

	void SetUniform(const std::string &name, glm::uint32 x, bool force = false) override final {
		auto it = uniformsU32.find(name);
		if(!force && it != uniformsU32.cend() && it->second == x) { return; }

		uniformsU32[name] = x;
		for(auto &node : nodes) {
			if(node.uniforms.find(name) != node.uniforms.end()) {
				GL(glUseProgram(node.program));
				UploadUniform(node.program, name, x);
			}
		}
	}

	void SetUniform(const std::string &name, Decimal x, bool force = false) override final {
		auto it = uniformsFloat.find(name);
		if(!force && it != uniformsFloat.cend() && it->second == x) { return; }

		uniformsFloat[name] = x;
		for(auto &node : nodes) {
			if(node.uniforms.find(name) != node.uniforms.end()) {
				GL(glUseProgram(node.program));
				UploadUniform(node.program, name, x);
			}
		}
	}

	void SetUniform(const std::string &name, Point3 p, bool force = false) override final {
		auto it = uniformsPoint3.find(name);
		if(!force && it != uniformsPoint3.cend() && it->second == p) { return; }

		uniformsPoint3[name] = p;
		for(auto &node : nodes) {
			if(node.uniforms.find(name) != node.uniforms.end()) {
				GL(glUseProgram(node.program));
				UploadUniform(node.program, name, p);
			}
		}
	}

	bool UploadUniform(GLuint program, const std::string &name, glm::int32 x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		GL(glUniform1i(loc, x));
		return true;
	}

	bool UploadUniform(GLuint program, const std::string &name, glm::uint32 x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		GL(glUniform1ui(loc, x));
		DebugManager()->Trace("uniform ", name, " = ", x);
		return true;
	}

	bool UploadUniform(GLuint program, const std::string &name, Decimal x) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto x2 = float(x);
		GL(glUniform1f(loc, x2));
		DebugManager()->Trace("uniform ", name, " = ", x);
		return true;
	}

	bool UploadUniform(GLuint program, const std::string &name, Point2 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto p2 = glm::vec2(p);
		GL(glUniform2f(loc, p2.x, p2.y));
		return true;
	}

	bool UploadUniform(GLuint program, const std::string &name, Point3 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto p2 = glm::vec3(p);
		GL(glUniform3f(loc, p2.x, p2.y, p2.z));
		return true;
	}

	bool UploadUniform(GLuint program, const std::string &name, Point4 p) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto p2 = glm::vec4(p);
		GL(glUniform4f(loc, p2.x, p2.y, p2.z, p2.w));
		return true;
	}

	bool UploadUniform(GLuint program, const std::string &name, Mat4 m) {
		GLint loc;
		GL(loc = glGetUniformLocation(program, name.c_str()));
		if(loc == -1) { return false; } // -1 if uniform does not exist in program
		auto m2 = glm::mat4(m);
		GL(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m2)));
		return true;
	}
};
