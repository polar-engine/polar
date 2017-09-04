#pragma once

#include <vector>
#include <memory>
#include <polar/core/destructor.h>
#include <polar/core/deltaticks.h>
#include <polar/component/base.h>

class Polar;

class System {
	friend class EngineState;
protected:
	std::vector<std::shared_ptr<Destructor>> dtors;
	Polar *engine;

	virtual void Init() {}
	virtual void Update(DeltaTicks &) {}
	virtual void ComponentAdded(IDType, const std::type_info *, std::weak_ptr<Component>) {}
	virtual void ComponentRemoved(IDType, const std::type_info *) {}
public:
	static bool IsSupported() { return false; }
	System(Polar *engine) : engine(engine) {}
	virtual ~System() {}
};
