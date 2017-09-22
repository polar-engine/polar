#pragma once

#include <vector>
#include <polar/util/gl.h>

namespace polar { namespace property { namespace gl32 {
	struct model : public base {
		GLuint vao;
		std::vector<GLuint> vbos;
		GLsizei numVertices = 0;
		GLsizei capacity = 0;

		inline friend bool operator<(const model &left, const model &right) {
			return left.capacity < right.capacity;
		}
	};
} } }