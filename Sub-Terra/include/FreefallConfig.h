#pragma once

#include <assert.h>
#include <iostream>
#include <string>

enum class SteamConfigOption {
	Grain,
	ScanIntensity,
	PixelFactor,
	VoxelFactor,
	Mute
};

std::istream & operator>>(std::istream &s, SteamConfigOption &x) {
	std::string word;
	s >> word;

#define CASE(X) (word == #X) { x = SteamConfigOption::X; }
	if CASE(Grain)
	else if CASE(ScanIntensity)
	else if CASE(PixelFactor)
	else if CASE(VoxelFactor)
	else if CASE(Mute)
	else { assert(false && "unhandled ConfigOption"); }
#undef CASE

	return s;
}

std::ostream & operator<<(std::ostream &s, const SteamConfigOption &x) {
#define CASE(X) case SteamConfigOption::X: s << #X; break;
	switch(x) {
	CASE(Grain)
	CASE(ScanIntensity)
	CASE(PixelFactor)
	CASE(VoxelFactor)
	CASE(Mute)
	default: assert(false && "unhandled ConfigOption");
	}
#undef CASE

	return s;
}

enum class LocalConfigOption {
	BaseDetail,
	Bloom,
	Cel
};

std::istream & operator>>(std::istream &s, LocalConfigOption &x) {
	std::string word;
	s >> word;

#define CASE(X) (word == #X) { x = LocalConfigOption::X; }
	if CASE(BaseDetail)
	else if CASE(Bloom)
	else if CASE(Cel)
	else { assert(false && "unhandled ConfigOption"); }
#undef CASE

	return s;
}

std::ostream & operator<<(std::ostream &s, const LocalConfigOption &x) {
#define CASE(X) case LocalConfigOption::X: s << #X; break;
	switch(x) {
	CASE(BaseDetail)
	CASE(Bloom)
	CASE(Cel)
	default: assert(false && "unhandled ConfigOption");
	}
#undef CASE

	return s;
}
