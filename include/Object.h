#pragma once

#include "Component.h"
#include "JobManager.h"
#include "EventManager.h"

class Object {
private:
	std::vector<Component *> _components;
public:
	JobManager *jobManager;
	EventManager *eventManager;
	Object() {}
	virtual ~Object();
	void AddComponent(Component *);
	virtual void Init();
	virtual void Update(int);
	virtual void Destroy();
};
