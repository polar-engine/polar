#pragma once

#include <string>

class System {
public:
	static bool IsSupported() { return false; }
	System() {}
	virtual ~System() {}
	virtual void Init() {}
	virtual void Update(float) {}
	virtual void Destroy() {}
};
