#pragma once

#include "Renderer.h"

class NoRenderer : public Renderer {
public:
	NoRenderer();
	virtual ~NoRenderer();
	void Init() override final;
};
