#pragma once

#include "JobManager.h"
#include "EventManager.h"

class Component {
public:
	JobManager *jobManager;
	EventManager *eventManager;
	Component() {}
	virtual ~Component() {};
	virtual void Init() {}
	virtual void Update(int) {}
	virtual void Destroy() {}
};
