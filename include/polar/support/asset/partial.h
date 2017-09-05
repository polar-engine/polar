#pragma once

#include <polar/asset/base.h>

namespace polar { namespace support { namespace asset {
	struct partial {
		bool done = false;
		std::string contents;
		std::shared_ptr<polar::asset::base> asset;
	};
}}}
