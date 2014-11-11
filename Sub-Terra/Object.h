#pragma once

#include <vector>
#include "Component.h"

class Object {
private:
	std::vector<Component *> _components;
public:
	Object() {}
	virtual ~Object();
	void AddComponent(Component *);
	virtual void Init();
	virtual void Update(float);
	virtual void Destroy();
};
