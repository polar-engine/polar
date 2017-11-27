#pragma once

#include <polar/asset/base.h>

namespace polar::support::asset {
	struct partial {
		bool done = false;
		std::string contents;
		std::shared_ptr<polar::asset::base> asset;
	};
} // namespace polar::support::asset
