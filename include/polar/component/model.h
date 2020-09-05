#pragma once

#include <polar/asset/model.h>
#include <polar/component/base.h>

namespace polar::component {
	class model : public base {
	  public:
		std::shared_ptr<asset::model> asset;
		core::ref stage;

		model(decltype(asset) asset, core::ref stage) : asset(asset), stage(stage) {}

		virtual std::string name() const override { return "model"; }
	};
} // namespace polar::component
