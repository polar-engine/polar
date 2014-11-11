#pragma once

#include "System.h"

class Renderer : public System {
public:
	static bool IsSupported() { return false; }
};
