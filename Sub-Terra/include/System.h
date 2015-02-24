#pragma once

#include "Object.h"

class Polar;

class System {
	friend class Polar;
protected:
	Polar *engine;
	virtual void Init() {}
	virtual void Update(DeltaTicks &) {}
	virtual void Destroy() {}
	virtual void ObjectAdded(Object *) {}
	virtual void ObjectRemoved(Object *) {}
public:
	static bool IsSupported() { return false; }
	System(Polar *engine) : engine(engine) {}
	virtual ~System() {}
};
