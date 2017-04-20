#pragma once

#include <assert.h>
#include <stdint.h>
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

/* prevent Windows.h from defining min/max macros */
#ifdef _WIN32
#define NOMINMAX
#endif

/* CF defines types Point and Component so it needs to be included early
 * also undef macro defined by Boost
 */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>

#ifdef check
#undef check
#endif

#endif

#include "types.h"
#include "debug.h"

#define ENGINE_TICKS_PER_SECOND 10000

typedef std::chrono::duration<uint64_t, std::ratio<1, ENGINE_TICKS_PER_SECOND>> DeltaTicksBase;
#include "DeltaTicks.h"

enum class GeometryType : uint8_t {
	None,
	Lines,
	Triangles,
	TriangleStrip
};

typedef std::uint_fast64_t IDType;

#include "Atomic.h"
#include "Polar.h"
