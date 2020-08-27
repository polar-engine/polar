#pragma once

#include <polar/asset/font.h>
#include <polar/math/types.h>
#include <polar/support/ui/credits.h>
#include <polar/support/ui/origin.h>
#include <polar/system/base.h>
#include <vector>

namespace polar::system {
	using credits_vector_t = std::vector<support::ui::credits_section>;

	class credits : public base {
		using origin_t = support::ui::origin;

	  private:
		credits_vector_t _credits;
		std::shared_ptr<polar::asset::font> font;
		math::decimal height = 0;

		void render_all();

	  protected:
		void init() override;
		void update(DeltaTicks &dt) override;

	  public:
		static bool supported() { return true; }
		credits(core::polar *engine, credits_vector_t _credits)
		    : base(engine), _credits(_credits) {}

		virtual std::string name() const override { return "credits"; }
	};
} // namespace polar::system
