#pragma once

#include <atomic>
#include <cstdint>
#include <polar/support/integrator/integrable.h>
#include <polar/support/sched/clock/integrator.h>
#include <polar/system/sched.h>

namespace polar::system {
	class integrator : public base {
	  private:
		DeltaTicks accumulator;
		void tick(DeltaTicks::seconds_type);

	  protected:
		void init() override {
			auto sch = engine->get<sched>().lock();
			keep(sch->bind<support::sched::clock::integrator>([this] (auto dt) {
				tick(dt.Seconds());
			}));
		}
	  public:
		static bool supported() { return true; }
		integrator(core::polar *engine) : base(engine) {}

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
