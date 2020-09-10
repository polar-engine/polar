#pragma once

#include <polar/component/base.h>
#include <polar/util/gl.h>

namespace polar::component::opengl {
	class framebuffer : public base {
	  public:
		struct data_type {
			GLuint fb;
			std::map<GLenum, GLuint> attachments;
		};
		using fbs_type = std::vector<data_type>;

		fbs_type fbs;

		size_t active = 0;

		framebuffer(fbs_type fbs) : fbs(fbs) {}
		framebuffer(data_type fb) : framebuffer(decltype(fbs){fb}) {}

		data_type current() const {
			return fbs[active];
		}

		data_type prev() const {
			auto idx = active
				? active - 1
				: fbs.size() - 1;
			return fbs[idx];
		}

		data_type next() const {
			auto idx = active + 1;
			if(idx >= fbs.size()) {
				idx = 0;
			}
			return fbs[idx];
		}

		data_type advance() {
			++active;
			if(active >= fbs.size()) {
				active = 0;
			}
			return current();
		}

		virtual std::string name() const override { return "opengl_framebuffer"; }
	};
} // namespace polar::component::opengl
