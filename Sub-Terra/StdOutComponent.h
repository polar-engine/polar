#pragma once

#include <iostream>
#include "Component.h"

class StdOutComponent : public Component {
private:
	std::string const _msg;
public:
	StdOutComponent(std::string const & msg) : _msg(msg) {}
	void Init() override final {
		jobManager->Do([this]() {
			std::cout << _msg << std::endl;
		});
	}
};
