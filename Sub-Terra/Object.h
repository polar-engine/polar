#pragma once

#include "Component.h"
#include "JobManager.h"

class Object {
private:
	std::vector<Component *> _components;
public:
	JobManager *jobManager;
	Object() {}
	virtual ~Object();
	void AddComponent(Component *);
	virtual void Init();
	virtual void Update(int);
	virtual void Destroy();
};
