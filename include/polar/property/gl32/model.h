#pragma once

#include <polar/util/gl.h>
#include <vector>

namespace polar {
	namespace property {
		namespace gl32 {
			struct model : public base {
				GLuint vao;
				std::vector<GLuint> vbos;
				GLsizei numVertices = 0;
				GLsizei capacity    = 0;

				GLuint diffuse_map;

				inline friend bool operator<(const model &left,
				                             const model &right) {
					return left.capacity < right.capacity;
				}
			};
		} // namespace gl32
	}     // namespace property
} // namespace polar
