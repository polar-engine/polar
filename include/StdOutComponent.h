#pragma once

#include "Component.h"

class StdOutComponent : public Component {
private:
	const std::string _msg;
public:
	StdOutComponent(const std::string &msg) : _msg(msg) {}
	void Init() override final {
		jobManager->Do([this] () {
			jobManager->Do([this] () {
				std::cout << _msg << std::endl;
			}, JobPriority::High, JobThread::Main);
		}, JobPriority::Low, JobThread::Worker);
	}
};
