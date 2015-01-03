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
#include <map>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>

/* CF defines types Point and Component so it needs to be included early */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#define GLM_SIMD_ENABLE_XYZW_UNION
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/simd_vec4.hpp"

#include "debug.h"

#define ENGINE_TICKS_PER_SECOND 10000

typedef std::chrono::duration<uint64_t, std::ratio<1, ENGINE_TICKS_PER_SECOND>> DeltaTicksBase;
#include "DeltaTicks.h"

typedef glm::fvec2 EnginePoint2;
typedef glm::fvec3 EnginePoint3;
typedef glm::fvec4 EnginePoint;
#define Point2 EnginePoint2
#define Point3 EnginePoint3
#define Point EnginePoint
typedef std::tuple<Point, Point, Point> EngineTriangle;
#define Triangle EngineTriangle

enum class GeometryType : uint8_t {
	None,
	Triangles,
	TriangleStrip
};

#include "Atomic.h"
#include "Polar.h"
#include "JobManager.h"
#include "EventManager.h"
#include "AssetManager.h"

#include "assets.h"
