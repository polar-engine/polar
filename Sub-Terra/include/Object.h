#pragma once

#include "EntityBase.h"
#include "Component.h"

typedef EntityBase<Component> EngineObject;
#define Object EngineObject
