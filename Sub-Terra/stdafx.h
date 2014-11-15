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

#include "Tag.h"
#include "System.h"
#include "JobManager.h"
#include "EventManager.h"
