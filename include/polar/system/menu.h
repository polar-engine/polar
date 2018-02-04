#pragma once

#include <polar/support/ui/menuitem.h>
#include <polar/system/action.h>
#include <polar/system/base.h>
#include <stdint.h>
#include <vector>

namespace polar::system {
	using menuitem_vector_t = std::vector<support::ui::menuitem>;

	class menu : public base {
	  private:
		menuitem_vector_t _menu;
		std::vector<size_t> stack;
		int current = 0;

		std::shared_ptr<polar::asset::font> font;
		std::vector<core::ref> itemDtors;
		std::unordered_map<int, core::ref> controlDtors;
		float selectionAlpha = 0.0f;

		// the size of the array determines how many concurrent sounds we can
		// play at once
		std::array<core::ref, 4> soundDtors;
		size_t soundIndex = 0;

		action::digital_ref a_up;
		action::digital_ref a_down;
		action::digital_ref a_right;
		action::digital_ref a_left;
		action::digital_ref a_forward;
		action::digital_ref a_backward;

		inline menuitem_vector_t *getcurrentmenu() {
			menuitem_vector_t *m = &_menu;
			for(auto i : stack) { m = &m->at(i).children; }
			return m;
		}

		void activate();
		void navigate(int down, int right = 0, bool force = false);
		void render(size_t i, bool replace = false);

	  protected:
		void init() override;
		inline void update(DeltaTicks &) override { render(current, true); }

	  public:
		Decimal uiScale = 0.3125;

		static bool supported() { return true; }
		menu(core::polar *engine, Decimal uiScale, menuitem_vector_t _menu)
		    : base(engine), _menu(_menu), uiScale(uiScale) {}

		const auto action_up()       { return a_up;       }
		const auto action_down()     { return a_down;     }
		const auto action_right()    { return a_right;    }
		const auto action_left()     { return a_left;     }
		const auto action_forward()  { return a_forward;  }
		const auto action_backward() { return a_backward; }

		inline void render_all() {
			auto m = getcurrentmenu();

			itemDtors.clear();
			controlDtors.clear();
			for(size_t i = 0; i < m->size(); ++i) { render(i); }
		}
	};
} // namespace polar::system
