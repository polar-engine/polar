#pragma once

class Component {
public:
	Component() {}
	virtual ~Component() {};
	virtual void Init() {}
	virtual void Update(float) {}
	virtual void Destroy() {}
};
