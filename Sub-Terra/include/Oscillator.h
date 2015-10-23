#pragma once

#include <math.h>
#include "WaveShape.h"

class Oscillator {
public:
	WaveShape waveShape;
	double frequency = 1.0;
	double amplitude = 1.0;
	double speed = 1.0;
	uint32_t phaseAccumulator = 0;

	Oscillator() = default;
	Oscillator(const WaveShape &waveShape) : waveShape(waveShape), frequency(waveShape.preferredFrequency) {}

	inline uint16_t Tick(const double sampleRate) {
		double frequencyOut = frequency * speed;
		uint32_t frequencyControlWord = frequencyOut * static_cast<double>(1ull << 32) / sampleRate;
		phaseAccumulator += frequencyControlWord;
		return amplitude * waveShape.buffer[(phaseAccumulator & 0xFF200000) >> 22];
	}
};
