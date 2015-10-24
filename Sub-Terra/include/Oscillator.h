#pragma once

#include "WaveShape.h"

#define LIGHTSPEED_MILES_PER_SECOND (186282.3970512)
#define SCIENTIFIC_A (sqrt(LIGHTSPEED_MILES_PER_SECOND))
#define MIDDLE_C (-9)
#define NOTE_TO_FREQUENCY(X) (SCIENTIFIC_A * std::pow(std::pow(2.0, 1.0 / 12.0), X))

class Oscillator {
protected:
	uint32_t phaseAccumulator = 0;
public:
	WaveShape waveShape;
	double frequency = 1.0;
	double amplitude = 1.0;

	Oscillator() = default;
	Oscillator(const WaveShape &waveShape, const double frequency = NOTE_TO_FREQUENCY(MIDDLE_C + 12)) : waveShape(waveShape), frequency(frequency) {}

	static constexpr uint32_t LowerMask(const unsigned int n) {
		return (n == 0)
			? 0
			: ((LowerMask(n - 1) << 1) | 1);
	}

	static constexpr uint32_t UpperMask(const unsigned int n) {
		return LowerMask(n) << (32 - n);
	}

	inline int16_t Tick(const double sampleRate) {
		uint32_t frequencyControlWord = static_cast<uint32_t>(frequency * static_cast<double>(1ull << 32) / sampleRate);
		phaseAccumulator += frequencyControlWord;
		return static_cast<int16_t>(amplitude * waveShape.table[(phaseAccumulator & UpperMask(WaveShape::granularity)) >> 22]);
	}
};
