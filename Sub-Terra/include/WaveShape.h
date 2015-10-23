#pragma once

#include <boost/array.hpp>

struct WaveShape {
public:
	static const unsigned int granularity = 10;
	static const uint16_t size = 1 << granularity;
	boost::array<uint16_t, size> table;
	double preferredFrequency;
};

inline WaveShape MkSineWaveShape() {
	WaveShape waveShape;
	for(unsigned int i = 0; i < WaveShape::size; ++i) {
		double sample = sin(i * 2.0 * 3.14159265358979 / WaveShape::size);
		waveShape.table[i] = static_cast<uint16_t>((sample + 1) * 32767.0 - 32768.0);
	}
	waveShape.preferredFrequency = 261.625565; /* middle C */
	return waveShape;
}
