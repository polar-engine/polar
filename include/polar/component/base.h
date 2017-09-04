#pragma once

#include <polar/core/ecsbase.h>
#include <polar/property/base.h>

typedef EntityBase<Property> EngineComponent;
#define Component EngineComponent

template<typename T> class TagComponent : public Component {};
