#pragma once

#include <assert.h>
#include <iostream>
#include <string>

namespace freefall {
	enum class CloudConfigOption {
		Grain,
		ScanIntensity,
		PixelFactor,
		VoxelFactor,
		Mute,
		MouseSmoothing,
		MasterVolume,
		MusicVolume,
		EffectVolume,
		ChromaticAberration
	};

	std::istream & operator>>(std::istream &s, CloudConfigOption &x) {
		std::string word;
		s >> word;

	#define CASE(X) (word == #X) { x = CloudConfigOption::X; }
		if CASE(Grain)
		else if CASE(ScanIntensity)
		else if CASE(PixelFactor)
		else if CASE(VoxelFactor)
		else if CASE(Mute)
		else if CASE(MouseSmoothing)
		else if CASE(MasterVolume)
		else if CASE(MusicVolume)
		else if CASE(EffectVolume)
		else if CASE(ChromaticAberration)
		else { assert(false && "unhandled ConfigOption"); }
	#undef CASE

		return s;
	}

	std::ostream & operator<<(std::ostream &s, const CloudConfigOption &x) {
	#define CASE(X) case CloudConfigOption::X: s << #X; break;
		switch(x) {
		CASE(Grain)
		CASE(ScanIntensity)
		CASE(PixelFactor)
		CASE(VoxelFactor)
		CASE(Mute)
		CASE(MouseSmoothing)
		CASE(MasterVolume)
		CASE(MusicVolume)
		CASE(EffectVolume)
		CASE(ChromaticAberration)
		default: assert(false && "unhandled ConfigOption");
		}
	#undef CASE

		return s;
	}

	enum class LocalConfigOption {
		BaseDetail,
		Bloom,
		Cel,
		Fullscreen,
		UIScale
	};

	std::istream & operator>>(std::istream &s, LocalConfigOption &x) {
		std::string word;
		s >> word;

	#define CASE(X) (word == #X) { x = LocalConfigOption::X; }
		if CASE(BaseDetail)
		else if CASE(Bloom)
		else if CASE(Cel)
		else if CASE(Fullscreen)
		else if CASE(UIScale)
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
		CASE(Fullscreen)
		CASE(UIScale)
		default: assert(false && "unhandled ConfigOption");
		}
	#undef CASE

		return s;
	}
}
