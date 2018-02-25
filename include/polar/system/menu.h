#pragma once

#include <polar/support/ui/menuitem.h>
#include <polar/system/base.h>
#include <stdint.h>
#include <vector>

namespace polar::system {
	using menuitem_t = support::ui::menuitem;
	using menuitem_vector_t = std::vector<menuitem_t>;

	class menu : public base {
	  private:
		menuitem_vector_t _menu;
		std::vector<size_t> stack;
		int current = 0;

		menuitem_t back_item = menuitem_t("Back", [this](Decimal) {
			navigate(0, -1, true);
			return false;
		});

		std::shared_ptr<polar::asset::font> font;
		std::vector<core::ref> itemDtors;
		std::unordered_map<int, core::ref> controlDtors;
		float selectionAlpha = 0.0f;

		// the size of the array determines how many concurrent sounds we can
		// play at once
		std::array<core::ref, 4> soundDtors;
		size_t soundIndex = 0;

		inline menuitem_vector_t & current_menu() {
			menuitem_vector_t *m = &_menu;
			for(auto i : stack) { m = &m->at(i).children; }
			return *m;
		}

		inline menuitem_t & current_at(size_t i) {
			auto &m = current_menu();
			if(i == m.size() && !stack.empty()) {
				return back_item;
			} else {
				return m.at(i);
			}
		}

		inline size_t current_size() {
			auto &m = current_menu();
			return stack.empty() ? m.size() : (m.size() + 1);
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
			auto size = current_size();

			itemDtors.clear();
			controlDtors.clear();
			for(size_t i = 0; i < size; ++i) { render(i); }
		}
	};
} // namespace polar::system
