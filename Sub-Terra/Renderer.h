#pragma once

class Renderer {
public:
	Renderer();
	virtual ~Renderer();
	virtual void Init() = 0;
};
