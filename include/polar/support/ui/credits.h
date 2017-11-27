#pragma once

#include <string>
#include <vector>

namespace polar::support::ui {
	class credits_section {
	  public:
		std::string value;
		std::vector<std::string> names;
		IDType id = INVALID_ID();
		std::vector<IDType> nameIDs;

		credits_section(std::string value, std::vector<std::string> names)
		    : value(value), names(names) {
			for(size_t i = 0; i < names.size(); ++i) {
				nameIDs.emplace_back(INVALID_ID());
			}
		}
	};
} // namespace polar::support::ui
