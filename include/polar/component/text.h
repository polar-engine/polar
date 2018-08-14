#pragma once

#include <polar/asset/font.h>
#include <polar/component/base.h>

namespace polar::component {
	class text : public base {
	  public:
		std::shared_ptr<asset::font> as;
		std::string str;
		bool fixedToViewport = false;

		text(std::shared_ptr<asset::font> as, std::string str)
		    : as(as), str(str) {}
	};
} // namespace polar::component
