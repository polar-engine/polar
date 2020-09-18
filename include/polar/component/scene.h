#pragma once

#include <polar/component/base.h>

namespace polar::component {
	class scene : public base {
	  public:
		bool serialize(core::store_serializer &s) const override {
			return true;
		}

		virtual std::string name() const override { return "scene"; }
	};
} // namespace polar::component
