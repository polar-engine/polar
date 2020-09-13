#pragma once

#include <polar/component/debug/timing.h>
#include <polar/system/base.h>

namespace polar::system::debug {
	class timing : public base {
	  private:
		std::chrono::high_resolution_clock::rep avg_count = 0;

		void update(DeltaTicks &) override {
			auto ref = engine->add();
			engine->add<component::debug::timing>(ref);

			std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();

			for(size_t i = 0; i < 100000; ++i) {
				auto timing = engine->get<component::debug::timing>(ref);
			}

			std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
			auto duration = end - begin;
			auto count = duration.count();

			if(avg_count == 0) {
				avg_count = count;
			} else {
				avg_count = count + 0.01 * (avg_count - count);
			}

			log()->notice("debug::timing", "average duration = ", avg_count);
		}

	  public:
		static bool supported() { return true; }

		timing(core::polar *engine) : base(engine) {}

		virtual std::string name() const override { return "debug_timing"; }
	};
} // namespace polar::system::debug
