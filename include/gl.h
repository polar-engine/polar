#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

inline bool _GL_real(const char *file, const long line, const char *code) {
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
		ENGINE_ERROR("OpenGL: 0x" << std::hex << err);
		ENGINE_DEBUG("    " << file << ':' << line);
		ENGINE_DEBUG("    " << code << '\n');
	}
	return err == GL_NO_ERROR;
}

#define GL(CODE)  ((CODE),  _GL_real(BASEFILE, __LINE__, #CODE))
