#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

#define GLM_SIMD_ENABLE_XYZW_UNION

#include <glm/glm.hpp>
#include <glm/gtx/simd_vec4.hpp>

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

#define OUTPUT(S) (std::cout << S)
#define ERROR(S)  (std::cerr << "========== ERROR ==========\n" << S << '\n')

#ifdef _DEBUG
#define DEBUG(S) OUTPUT(S << '\n')
#define CONTINUE ((std::cout << "Press enter to continue."), (std::cin.ignore(1)))
#else
#define DEBUG(S)
#define CONTINUE
#endif

union Arg {
	float float_;
	void *pVoid;

	Arg(float f) { float_ = f; }
	Arg(std::nullptr_t) { pVoid = nullptr; }
	template<typename T> Arg(T *p) { pVoid = reinterpret_cast<void *>(p); }
	template<typename T> T * Get() { return reinterpret_cast<T *>(pVoid); }
};

#include "Tag.h"
#include "System.h"
#include "JobManager.h"
#include "EventManager.h"
