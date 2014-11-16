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

/* TODO: use GLM types instead because who doesn't like SIMD :D */
template<typename T> using Tuple2 = std::tuple<T, T>;
template<typename T> using Tuple3 = std::tuple<T, T, T>;
template<typename T> using Tuple4 = std::tuple<T, T, T, T>;

union Arg {
	void *v;
	float *f;
	Tuple2<Arg> *t2;
	Tuple3<Arg> *t3;
	Tuple4<Arg> *t4;
	template<typename T> Arg(T p) { v = reinterpret_cast<void *>(p); }
	template<typename T> T Get() { return reinterpret_cast<T>(v); }
};

#include "Tag.h"
#include "System.h"
#include "JobManager.h"
#include "EventManager.h"
