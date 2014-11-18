#pragma once

#include <assert.h>
#include <iostream>
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

const char pathSeparator =
#ifdef _WIN32
'\\'
#else
'/'
#endif
;

inline const char *basename(const char *path) {
	const char *r = strrchr(path, pathSeparator);
	return r ? r + 1 : path;
}

#define BASEFILE (basename(__FILE__))

#define ENGINE_OUTPUT(S) (std::cout << S)
#define ENGINE_ERROR(S)  (std::cerr << "========== ERROR ==========\n" << S << '\n')

#ifdef _DEBUG
#define ENGINE_DEBUG(S) ENGINE_OUTPUT(S << '\n')
#define ENGINE_CONTINUE ((std::cout << "Press enter to continue."), (std::cin.ignore(1)))
#else
#define ENGINE_DEBUG(S)
#define ENGINE_CONTINUE
#endif

union Arg {
	float float_;
	void *pVoid;

	Arg(float f) { float_ = f; }
	Arg(std::nullptr_t) { pVoid = nullptr; }
	template<typename T> Arg(T *p) { pVoid = reinterpret_cast<void *>(p); }
	template<typename T> T * Get() { return reinterpret_cast<T *>(pVoid); }
};

#define ENGINE_TICKS_PER_SECOND 100

typedef std::chrono::duration<uint64_t, std::ratio<1, ENGINE_TICKS_PER_SECOND>> DeltaTicks;

typedef glm::simdVec4 Point;
typedef std::tuple<Point, Point, Point> Triangle;

#include "Tag.h"
#include "Object.h"
#include "System.h"
#include "JobManager.h"
#include "EventManager.h"
