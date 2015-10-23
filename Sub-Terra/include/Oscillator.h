#pragma once

#include <math.h>
#include "WaveShape.h"

class Oscillator {
public:
	WaveShape waveShape;
	double frequency = 1.0;
	double amplitude = 1.0;
	double speed = 1.0;
	double position = 0.0;

	Oscillator() = default;
	Oscillator(const WaveShape &waveShape) : waveShape(waveShape), frequency(waveShape.preferredFrequency) {}

	inline double Tick(const double sampleRate) {
		double x = floor(position);
		double y = waveShape.buffer[static_cast<size_t>(x)];

		double increment = frequency * speed * waveShape.buffer.size() / sampleRate;
		position += increment;
		for(; position >= waveShape.buffer.size(); position -= waveShape.buffer.size()) {}
		for(; position < 0; position += waveShape.buffer.size()) {}

		return y * amplitude;
	}
};
