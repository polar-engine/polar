#pragma once

class Polar;

class System {
	friend class Polar;
protected:
	const Polar *engine;
	virtual void Init() {}
	virtual void Update(DeltaTicks &, std::vector<Object *> &) {}
	virtual void Destroy() {}
	virtual void ObjectAdded(Object *) {}
public:
	static bool IsSupported() { return false; }
	System(const Polar *engine) : engine(engine) {}
	virtual ~System() {}
};
