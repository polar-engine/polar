#pragma once

#ifdef _MSC_VER
#define PACK(DECL) __pragma(pack(push, 1)) DECL __pragma(pack(pop))
#else
#define PACK(DECL) DECL __attribute__((__packed__))
#endif
