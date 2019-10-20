#pragma once

#include <polar/component/sprite/base.h>

namespace polar::component::sprite {
	class box : public base {
	  public:
		virtual std::string name() const override { return "sprite_box"; }

		void render_me() override;
	};
} // namespace polar::component::sprite
