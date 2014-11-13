#pragma once

#include "JobManager.h"

class Component {
public:
	JobManager *jobManager;
	Component() {}
	virtual ~Component() {};
	virtual void Init() {}
	virtual void Update(int) {}
	virtual void Destroy() {}
};
