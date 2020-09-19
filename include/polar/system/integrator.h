#pragma once

#include <atomic>
#include <cstdint>
#include <polar/component/clock/simulation.h>
#include <polar/component/listener.h>
#include <polar/support/integrator/integrable.h>
#include <polar/tag/clock/simulation.h>

namespace polar::system {
	class integrator : public base {
	  private:
		DeltaTicks accumulator;
		void tick(DeltaTicks::seconds_type);

	  protected:
		void init() override {
			auto clock = engine.own<tag::clock::simulation>();
			engine.add_as<component::clock::base, component::clock::simulation>(clock);

			core::ref listener;
			keep(listener = engine.add());
			engine.add<component::listener>(listener, clock, [this](auto dt) {
				tick(dt.Seconds());
			});
		}
	  public:
		static bool supported() { return true; }
		integrator(core::polar &engine) : base(engine) {}

		virtual std::string name() const override { return "integrator"; }

		virtual accessor_list accessors() const override {
			accessor_list l;
			/*
			l.emplace_back("fps", make_accessor<integrator>(
				[] (integrator *ptr) {
					return ptr->fps;
				},
				[] (integrator *ptr, auto x) {
					ptr->fps = x;
					ptr->timestep = DeltaTicks(ENGINE_TICKS_PER_SECOND / x);
				}
			));
			*/
			return l;
		}

		void revert_by(size_t = 1);
		void revert_to(size_t = 0);
	};
} // namespace polar::system
