#pragma once

#include <unordered_map>
#include <polar/util/gl.h>

namespace polar { namespace support { namespace gl32 {
	struct pipelinenode {
		GLuint program;
		GLuint fbo = 0;
		std::unordered_set<std::string> uniforms;
		std::unordered_map<std::string, GLuint> outs;
		std::unordered_map<std::string, std::string> ins;
		std::unordered_map<std::string, GLuint> globalOuts;
		std::unordered_map<std::string, std::string> globalIns;
		pipelinenode(GLuint program) : program(program) {}
	};
} } }
