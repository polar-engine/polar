#pragma once

#include <polar/asset/image.h>
#include <polar/component/base.h>

namespace polar::component {
	class texture : public base {
	  public:
		std::shared_ptr<asset::image> asset;

		texture(decltype(asset) asset) : asset(asset) {}

		virtual std::string name() const override { return "texture"; }
	};
} // namespace polar::component
