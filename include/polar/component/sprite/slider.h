#pragma once

#include <polar/component/sprite/base.h>

namespace polar {
namespace component {
	namespace sprite {
		class slider : public base {
		  private:
			const float width;
			const float height;
			const float alpha;

		  public:
			slider(const float width, const float height, const float alpha)
			    : width(width), height(height), alpha(alpha) {}

			void render_me() override final;
		};
	}
}
}
