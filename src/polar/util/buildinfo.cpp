#include <polar/util/buildinfo.h>

const char * buildinfo_date() {
	return __DATE__;
}

const char * buildinfo_time() {
	return __TIME__;
}
