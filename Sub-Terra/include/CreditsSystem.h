#pragma once

#include <vector>
#include "System.h"

class CreditsSection {
public:
	std::string name;

	CreditsSection(std::string name, std::vector<std::string>) : name(name) {}
};

using Credits = std::vector<CreditsSection>;

class CreditsSystem : public System {

};
