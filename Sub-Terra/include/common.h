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

/* CF defines types Point and Component so it needs to be included early */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_SIMD_ENABLE_XYZW_UNION
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/simd_vec4.hpp"

#include "debug.h"

#define ENGINE_TICKS_PER_SECOND 10000

typedef std::chrono::duration<uint64_t, std::ratio<1, ENGINE_TICKS_PER_SECOND>> DeltaTicks;

typedef glm::fvec4 EnginePoint;
#define Point EnginePoint
typedef std::tuple<Point, Point, Point> EngineTriangle;
#define Triangle EngineTriangle

#include "Polar.h"
#include "JobManager.h"
#include "EventManager.h"
#include "AssetManager.h"

#include "assets.h"
