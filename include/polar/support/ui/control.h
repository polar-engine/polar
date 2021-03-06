#pragma once

#include <polar/component/color.h>
#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/sprite/box.h>
#include <polar/component/sprite/slider.h>
#include <polar/core/polar.h>

namespace polar::support::ui::control {
	class base {
	  public:
		virtual ~base() {}
		virtual float get() { return 0; }
		virtual bool activate() { return false; }
		virtual bool navigate(int) { return false; }
		virtual void render(core::polar *, core::ref, math::point2, float) {
		}
	};

	class button : public base {
	  public:
		button() {}
		bool activate() override { return true; }
	};

	class checkbox : public base {
	  private:
		bool state;

	  public:
		checkbox(bool initial = false) : state(initial) {}
		float get() override { return state; }

		bool activate() override {
			state = !state;
			return true;
		}

		bool navigate(int delta) override {
			// flip state delta times
			state ^= delta & 1;
			return true;
		}

		void render(core::polar *engine, core::ref object, math::point2 origin, float scale) override {
			math::decimal pad = 15;
			math::point2 offset = math::point2(4 * scale, 0); // edgeOffset + edgePadding (SliderSprite)s
			math::point4 color = state ? math::point4(0, 1, 0, 1) : math::point4(1, 0, 0, 1);

			engine->add_as<component::sprite::base, component::sprite::box>(object);
			engine->add<component::screenposition>(object, origin + pad + offset);
			engine->add<component::scale>(object, math::point3(scale));
			engine->add<component::color>(object, color);
		}
	};

	template<typename T> class slider : public base {
	  private:
		T min;
		T max;
		T value;
		T step;

	  public:
		slider(T min, T max, T initial = 0, T step = 1)
		    : min(min), max(max), value(initial), step(step) {}
		float get() override { return value; }
		bool activate() override { return navigate(1); }

		bool navigate(int delta) override {
			T newValue   = glm::clamp(value + T(delta) * step, min, max);
			bool changed = newValue != value;
			value        = newValue;
			return changed;
		}

		void render(core::polar *engine, core::ref object, math::point2 origin, float scale) override {
			math::decimal pad = 15;
			float alpha = float(value - min) / float(max - min);

			engine->add_as<component::sprite::base, component::sprite::slider>(object, 12 * 8, 12, alpha);
			engine->add<component::screenposition>(object, origin + pad);
			engine->add<component::scale>(object, math::point3(scale));
		}
	};
} // namespace polar::support::ui::control
