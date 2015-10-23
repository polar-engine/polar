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

	inline uint16_t Tick(const double sampleRate) {
		double size = static_cast<double>(waveShape.buffer.size());
		double x = floor(position);
		uint16_t y = waveShape.buffer[static_cast<size_t>(x)];

		double increment = frequency * speed * size / sampleRate;
		position += increment;

		for(; position >= size; position -= size) {}
		for(; position < 0; position += size) {}

		return static_cast<uint16_t>(static_cast<double>(y) * amplitude);
	}
};
