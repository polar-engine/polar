#pragma once

#include "EntityBase.h"
#include "Property.h"

typedef EntityBase<Property> EngineComponent;
#define Component EngineComponent
