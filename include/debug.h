#pragma once

#include <iostream>

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

#define INFO(msg) (std::cout << "[INFO] " << (msg) << std::endl)
#define INFOS(msg) (std::cout << "[INFO] " << msg << std::endl)

#define ENGINE_OUTPUT(S)  (std::cout << S)
#define ENGINE_ERROR(S) (std::cerr << "========== ERROR ==========\n" << S << '\n')
#define ENGINE_THROW(S) (ENGINE_ERROR(S), throw std::runtime_error(S))

#ifdef _DEBUG
#define ENGINE_DEBUG(S) ENGINE_OUTPUT(S << '\n')
#define ENGINE_CONTINUE (ENGINE_DEBUG("Press enter to continue."), (std::cin.ignore(1)), ENGINE_DEBUG(""))
#define ENGINE_NDEBUGERROR(E, D) (ENGINE_ERROR(E), ENGINE_DEBUG(D), ENGINE_DEBUG(""))
#define ENGINE_DEBUGERROR(E, D) (ENGINE_NDEBUGERROR(E, D), ENGINE_CONTINUE)
#else
#define ENGINE_CONTINUE
#define ENGINE_DEBUG
#define ENGINE_NDEBUGERROR(E, D) ENGINE_ERROR(E)
#define ENGINE_DEBUGERROR ENGINE_NDEBUGERROR
#endif
