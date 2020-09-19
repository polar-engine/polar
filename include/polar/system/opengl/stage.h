#pragma once

#include <polar/component/opengl/stage.h>
#include <polar/component/stage.h>
#include <polar/system/base.h>
#include <polar/util/gl.h>

namespace polar::system::opengl {
	class stage : public base {
	  private:
		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::stage)) {
				auto comp = std::static_pointer_cast<component::stage>(ptr.lock());

				auto win = engine->get<component::opengl::window>(comp->win);
				SDL(SDL_GL_MakeCurrent(win->win, win->ctx));

				log()->verbose("opengl::stage", "building shader program");

				auto program_id = build_program(comp);
				engine->add<component::opengl::stage>(wr, program_id);
			}
		}

		void component_removed(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::stage)) {
				auto stage = engine->get<component::stage>(wr);
				auto win = engine->get<component::opengl::window>(stage->win);
				SDL(SDL_GL_MakeCurrent(win->win, win->ctx));

				auto comp = std::static_pointer_cast<component::opengl::stage>(ptr.lock());
				GL(glDeleteProgram(comp->program));
			}
		}
		GLuint build_program(std::shared_ptr<component::stage> as) {
			using shadertype = support::shader::shadertype;

			std::vector<GLuint> ids;
			for(auto &shader : as->shaders) {
				GLenum type = 0;
				switch(shader.type) {
				case shadertype::vertex:
					type = GL_VERTEX_SHADER;
					break;
				case shadertype::fragment:
					type = GL_FRAGMENT_SHADER;
					break;
				case shadertype::invalid:
					log()->fatal("opengl::stage", "invalid shader type");
				}

				GLuint id;
				if(!GL(id = glCreateShader(type))) { log()->fatal("opengl::stage", "failed to create shader"); }

				const GLchar *src = shader.source.c_str();
				const GLint len   = static_cast<GLint>(shader.source.length());
				if(!GL(glShaderSource(id, 1, &src, &len))) { log()->fatal("opengl::stage", "failed to upload shader source"); }
				if(!GL(glCompileShader(id))) { log()->fatal("opengl::stage", "shader compilation is unsupported on this platform"); }

				GLint status;
				if(!GL(glGetShaderiv(id, GL_COMPILE_STATUS, &status))) {
					log()->fatal("opengl::stage", "failed to get shader compilation status");
				}

				if(status == GL_FALSE) {
					GLint infoLen;
					if(!GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLen))) {
						log()->fatal("opengl::stage", "failed to get shader info log length");
					}

					if(infoLen > 0) {
						auto infoLog = std::unique_ptr<char[]>(new char[infoLen]);
						if(!GL(glGetShaderInfoLog(id, infoLen, NULL, infoLog.get()))) {
							log()->fatal("opengl::stage", "failed to get shader info log");
						}
						log()->debug("opengl::stage", "shader info log:");
						log()->debug("opengl::stage", infoLog.get());
					}
					log()->fatal("opengl::stage", "failed to compile shader");
				}

				ids.push_back(id);
			}

			GLuint programID;
			if(!GL(programID = glCreateProgram())) { log()->fatal("opengl::stage", "failed to create program"); }

			for(auto id : ids) {
				if(!GL(glAttachShader(programID, id))) { log()->fatal("opengl::stage", "failed to attach shader to program"); }

				// flag shader for deletion
				if(!GL(glDeleteShader(id))) { log()->fatal("opengl::stage", "failed to flag shader for deletion"); }
			}

			if(!GL(glLinkProgram(programID))) { log()->fatal("opengl::stage", "program linking is unsupported on this platform"); }

			GLint status;
			if(!GL(glGetProgramiv(programID, GL_LINK_STATUS, &status))) {
				log()->fatal("opengl::stage", "failed to get program linking status");
			}

			if(status == GL_FALSE) {
				GLint infoLen;
				if(!GL(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen))) {
					log()->fatal("opengl::stage", "failed to get program info log length");
				}

				if(infoLen > 0) {
					auto infoLog = std::unique_ptr<char[]>(new char[infoLen]);
					if(!GL(glGetProgramInfoLog(programID, infoLen, NULL, infoLog.get()))) {
						log()->fatal("opengl::stage", "failed to get program info log");
					}
					log()->debug("opengl::stage", "program info log:");
					log()->debug("opengl::stage", infoLog.get());
				}
				log()->fatal("opengl::stage", "failed to link program");
			}
			return programID;
		}

	  public:
		static bool supported() { return true; }

		stage(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "opengl_stage"; }
	};
} // namespace polar::system::opengl
