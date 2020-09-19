#pragma once

#include <polar/component/opengl/model.h>
#include <polar/system/base.h>

namespace polar::system::opengl {
	class model : public base {
	  private:
		void component_added(core::weak_ref wr, std::type_index ti, std::weak_ptr<component::base> ptr) {
			if(ti == typeid(component::model)) {
				auto model = std::static_pointer_cast<component::model>(ptr.lock());

				GLsizei count   = GLsizei(model->triangles.size()) * 3;
				GLsizeiptr size = model->triangles.size() * sizeof(component::model::triangle);
				void *data      = model->triangles.data();

				GLuint vao;
				GL(glGenVertexArrays(1, &vao));
				GL(glBindVertexArray(vao));

				/* location   attribute
				 *
				 *        0   vertex
				 *        1   normal
				 *        2   texcoord
				 */

				GLuint buffer;
				GL(glGenBuffers(1, &buffer));

				GL(glBindBuffer(GL_ARRAY_BUFFER, buffer));

				const GLsizei stride = sizeof(component::model::vertex);
				const GLvoid *p_ptr  = NULL;
				const GLvoid *n_ptr  = (GLvoid *)sizeof(math::point3);
				const GLvoid *t_ptr  = (GLvoid *)(sizeof(math::point3) * 2);

				GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, p_ptr));
				GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, n_ptr));
				GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, t_ptr));

				GL(glEnableVertexAttribArray(0));
				GL(glEnableVertexAttribArray(1));
				GL(glEnableVertexAttribArray(2));

				GL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));

				engine.add<component::opengl::model>(wr, vao, buffer, count);
			}
		}

		void component_removed(core::weak_ref, std::type_index ti, std::weak_ptr<component::base> ptr) override {
			if(ti == typeid(component::opengl::model)) {
				auto comp = std::static_pointer_cast<component::opengl::model>(ptr.lock());

				GL(glDeleteBuffers(1, &comp->buffer));
				GL(glDeleteVertexArrays(1, &comp->vao));
			}
		}

	  public:
		static bool supported() { return true; }

		model(core::polar &engine) : base(engine) {}

		virtual std::string name() const override { return "opengl_model"; }
	};
} // namespace polar::system::opengl
