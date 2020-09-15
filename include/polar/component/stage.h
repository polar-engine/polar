#pragma once

#include <polar/asset/shaderprogram.h>
#include <polar/component/base.h>
#include <polar/core/ref.h>

namespace polar::component {
	class stage : public base {
	  public:
		std::shared_ptr<asset::shaderprogram> asset;

		stage(decltype(asset) asset) : asset(asset) {}

		virtual std::string name() const override { return "stage"; }
	};
} // namespace polar::component
