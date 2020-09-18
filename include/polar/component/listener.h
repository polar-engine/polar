#pragma once

#include <polar/core/deltaticks.h>
#include <polar/core/ref.h>

namespace polar::component {
	class listener : public base {
	  public:
		using handler_type = std::function<void(DeltaTicks)>;

	  private:
		core::ref r;
		handler_type h;

	  public:
		listener(core::ref r, handler_type h) : r(r), h(h) {}

		auto ref() const {
			return r;
		}

		void trigger(DeltaTicks &dt) const {
			h(dt);
		}

		bool serialize(core::store_serializer &s) const override {
			return false;
		}

		std::string name() const override { return "listener"; }
	};
} // namespace polar::component
