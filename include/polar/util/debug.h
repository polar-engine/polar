#pragma once

#include <iostream>
#include <string.h>

const char pathSeparator =
#if defined(_WIN32)
    '\\'
#else
    '/'
#endif
    ;

inline const char *polar_basename(const char *path) {
	const char *r = strrchr(path, pathSeparator);
	return r ? r + 1 : path;
}

#define BASEFILE (polar_basename(__FILE__))
