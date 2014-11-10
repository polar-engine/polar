#pragma once

#include "Renderer.h"

class Polar {
private:
	Renderer &_renderer;
public:
	Polar(Renderer &);
	virtual ~Polar();
	void Init();
};
