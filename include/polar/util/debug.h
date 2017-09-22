#pragma once

#include <string.h>
#include <iostream>

const char pathSeparator =
#if defined(_WIN32)
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
