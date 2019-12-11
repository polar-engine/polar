#pragma once

#include <string>
#include <vector>

namespace polar::support::ui {
	class credits_section {
	  public:
		std::string value;
		std::vector<std::string> names;
		core::ref object;
		std::vector<core::ref> name_objects;

		credits_section(std::string value, std::vector<std::string> names) : value(value), names(names) {
			for(size_t i = 0; i < names.size(); ++i) {
				name_objects.emplace_back();
			}
		}
	};
} // namespace polar::support::ui
