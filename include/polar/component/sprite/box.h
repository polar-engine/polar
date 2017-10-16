#pragma once

#include <polar/component/sprite/base.h>

namespace polar {
namespace component {
	namespace sprite {
		class box : public base {
		  public:
			void render_me() override final;
		};
	}
}
}
