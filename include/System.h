#pragma once

class JobManager;
class EventManager;

class System {
public:
	JobManager *jobManager;
	EventManager *eventManager;
	static bool IsSupported() { return false; }
	System() {}
	virtual ~System() {}
	virtual void Init() {}
	virtual void Update(DeltaTicks &, std::vector<Object *> &) {}
	virtual void Destroy() {}
	virtual void ObjectAdded(Object *) {}
};
