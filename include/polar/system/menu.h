#pragma once

#include <polar/support/ui/menuitem.h>
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
		std::vector<std::shared_ptr<core::destructor>> itemDtors;
		std::unordered_map<int, std::shared_ptr<core::destructor>> controlDtors;
		float selectionAlpha = 0.0f;

		// the size of the array determines how many concurrent sounds we can
		// play at once
		std::array<std::shared_ptr<core::destructor>, 4> soundDtors;
		size_t soundIndex = 0;

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

		inline void render_all() {
			auto m = getcurrentmenu();

			itemDtors.clear();
			controlDtors.clear();
			for(size_t i = 0; i < m->size(); ++i) { render(i); }
		}
	};
} // namespace polar::system
