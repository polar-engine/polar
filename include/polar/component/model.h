#pragma once

#include <polar/asset/model.h>
#include <polar/component/base.h>

namespace polar::component {
	class model : public base {
	  public:
		std::shared_ptr<asset::model> asset;

		model(decltype(asset) asset) : asset(asset) {}

		virtual std::string name() const override { return "model"; }
	};
} // namespace polar::component
