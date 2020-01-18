#pragma once

#include <array>
#include <polar/math/constants.h>

class WaveShape {
  public:
	static const unsigned int granularity = 12;
	static const int16_t size             = 1 << granularity;
	std::array<int16_t, size> table       = {{0}};
};

inline WaveShape MkSineWaveShape() {
	WaveShape waveShape;
	for(unsigned int i = 0; i < WaveShape::size; ++i) {
		double sample = sin(i * polar::math::TWO_PI / WaveShape::size);
		waveShape.table[i] =
		    static_cast<int16_t>((sample + 1) * 32767.0 - 32768.0);
	}
	return waveShape;
}

inline WaveShape MkSquareWaveShape() {
	WaveShape waveShape;
	for(unsigned int i = 0; i < WaveShape::size; ++i) {
		double sample = i * 2 < WaveShape::size ? 1.0 : -1.0;
		waveShape.table[i] =
		    static_cast<int16_t>((sample + 1) * 32767.0 - 32768.0);
	}
	return waveShape;
}
