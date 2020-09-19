#pragma once

#include <polar/component/opengl/window.h>

namespace polar::system::opengl {
	class debug : public base {
	  private:
		static void GLAPIENTRY debug_cb(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		                                const GLchar *message, const void *userParam) {
			(void)source;
			(void)type;
			(void)id;
			(void)severity;
			(void)length;
			(void)userParam;
			log()->debug("opengl::debug", "OPENGL DEBUG OUTPUT: ", message);
		}

		void component_added(core::weak_ref, std::type_index ti, std::weak_ptr<component::base>) {
			if(ti == typeid(component::opengl::window)) {
				if(glewIsExtensionSupported("GL_KHR_debug")) {
					GL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
					GL(glDebugMessageCallback(debug_cb, nullptr));
					GL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE));
				}
			}
		}
	  public:
		static bool supported() { return true; }

		debug(core::polar &engine) : base(engine) {}

		virtual std::string name() const override { return "opengl_debug"; }
	};
} // namespace polar::system::opengl
