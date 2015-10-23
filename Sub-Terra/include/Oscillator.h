#pragma once

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

	static constexpr uint32_t LowerMask(unsigned int n) {
		return (n == 0)
			? 0
			: ((LowerMask(n - 1) << 1) | 1);
	}

	static constexpr uint32_t UpperMask(unsigned int n) {
		return LowerMask(n) << (32 - n);
	}

	inline uint16_t Tick(const double sampleRate) {
		double frequencyOut = frequency * speed;
		uint32_t frequencyControlWord = static_cast<uint32_t>(frequencyOut * static_cast<double>(1ull << 32) / sampleRate);
		phaseAccumulator += frequencyControlWord;
		return static_cast<uint32_t>(amplitude * waveShape.table[(phaseAccumulator & UpperMask(WaveShape::granularity)) >> 22]);
	}
};
