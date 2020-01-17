#pragma once

#include <GL/glew.h>
#include <polar/core/log.h>
#include <polar/util/debug.h>

inline bool _GL_real(const char *file, const long line, const char *code) {
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
		polar::log()->warning("OpenGL: ", err);
		polar::log()->debug(file, ' ', line);
		polar::log()->trace(code);
	}
	return err == GL_NO_ERROR;
}

#define ENGINE_GL(CODE) ((CODE), _GL_real(BASEFILE, __LINE__, #CODE))

#ifdef _DEBUG
#define GL ENGINE_GL
#define IGNORE_GL(CODE) ((CODE), glGetError());
#else
#define GL(CODE) ((CODE), true)
#define IGNORE_GL GL
#endif
