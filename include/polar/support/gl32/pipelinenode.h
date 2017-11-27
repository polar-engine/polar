#pragma once

#include <polar/util/gl.h>
#include <unordered_map>
#include <unordered_set>

namespace polar::support::gl32 {
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
} // namespace polar::support::gl32
