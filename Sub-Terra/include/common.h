#pragma once

#include <assert.h>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>
#include <typeinfo>
#include <array>
#include <vector>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

#define GLM_SIMD_ENABLE_XYZW_UNION
#include "glm/glm.hpp"
#include "glm/gtx/simd_vec4.hpp"

#include "debug.h"

#define ENGINE_TICKS_PER_SECOND 10000

typedef std::chrono::duration<uint64_t, std::ratio<1, ENGINE_TICKS_PER_SECOND>> DeltaTicks;

typedef glm::fvec4 Point;
typedef std::tuple<Point, Point, Point> Triangle;

#include "Tag.h"
#include "EntityBase.h"
#include "Component.h"

typedef EntityBase<Component> Object;

#include "System.h"
#include "Polar.h"
#include "JobManager.h"
#include "EventManager.h"
#include "AssetManager.h"

#include "assets.h"
