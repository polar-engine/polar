#pragma once

class EngineComponent {
public:
	virtual ~EngineComponent() {};
};

#define Component EngineComponent
