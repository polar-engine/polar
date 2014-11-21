#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

inline bool _GL_real(const char *file, const long line, const char *code) {
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
		ENGINE_DEBUGERROR("OpenGL: 0x" << std::hex << err,
						  "    " << file << ' ' << line << '\n' <<
						  "    " << code);
	}
	return err == GL_NO_ERROR;
}

#define ENGINE_GL(CODE) ((CODE),  _GL_real(BASEFILE, __LINE__, #CODE))

#ifdef _DEBUG
#define GL ENGINE_GL
#define IGNORE_GL(CODE) ((CODE), glGetError());
#else
#define GL(CODE) ((CODE), true)
#define IGNORE_GL GL
#endif
