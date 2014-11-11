#pragma once

#include "Renderer.h"

class NoRenderer : public Renderer {
public:
	static bool IsSupported() { return true; }
	void Destroy() override final;
};
