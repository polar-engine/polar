#pragma once

#include <array>
#include <iostream>
#include <polar/core/serializer.h>
#include <polar/core/types.h>

namespace polar::support::level {
	using polar::core::deserializer;
	using polar::core::serializer;

	struct keyframe {
		uint64_t ticks;
		Decimal baseThreshold;
		Decimal beatTicks;
		Decimal beatPower;
		Decimal beatStrength;
		Decimal waveTicks;
		Decimal wavePower;
		Decimal waveStrength;
		Point3 worldScale{1};
		uint64_t colorTicks;
		std::array<Point3, 3> colors;

		explicit keyframe(uint64_t t = 0) : ticks(t) {}
		explicit keyframe(uint64_t t, const keyframe &kf) : keyframe(kf) {
			ticks = t;
		}

		friend std::ostream &operator<<(std::ostream &s, const keyframe &kf) {
			s << "ticks         = " << kf.ticks << std::endl;
			s << "baseThreshold = " << kf.baseThreshold << std::endl;
			s << "beatTicks     = " << kf.beatTicks << std::endl;
			s << "beatPower     = " << kf.beatPower << std::endl;
			s << "beatStrength  = " << kf.beatStrength << std::endl;
			s << "waveTicks     = " << kf.waveTicks << std::endl;
			s << "wavePower     = " << kf.wavePower << std::endl;
			s << "waveStrength  = " << kf.waveStrength << std::endl;
			s << "colorTicks    = " << kf.colorTicks << std::endl;
			return s;
		}

		friend bool operator<(const keyframe &a, const keyframe &b) {
			return a.ticks < b.ticks;
		}

		friend keyframe operator+(const keyframe &lhs, const keyframe &rhs) {
			keyframe kf(lhs.ticks);
			kf.baseThreshold = lhs.baseThreshold + rhs.baseThreshold;
			kf.beatTicks     = lhs.beatTicks + rhs.beatTicks;
			kf.beatPower     = lhs.beatPower + rhs.beatPower;
			kf.beatStrength  = lhs.beatStrength + rhs.beatStrength;
			kf.waveTicks     = lhs.waveTicks + rhs.waveTicks;
			kf.wavePower     = lhs.wavePower + rhs.wavePower;
			kf.waveStrength  = lhs.waveStrength + rhs.waveStrength;
			kf.worldScale    = lhs.worldScale + rhs.worldScale;
			kf.colorTicks    = lhs.colorTicks + rhs.colorTicks;
			for(size_t i = 0; i < kf.colors.size(); ++i) {
				kf.colors[i] = lhs.colors[i] + rhs.colors[i];
			}
			return kf;
		}

		friend keyframe operator*(const keyframe &lhs, const Decimal x) {
			keyframe kf(lhs.ticks);
			kf.baseThreshold = lhs.baseThreshold * x;
			kf.beatTicks     = lhs.beatTicks * x;
			kf.beatPower     = lhs.beatPower * x;
			kf.beatStrength  = lhs.beatStrength * x;
			kf.waveTicks     = lhs.waveTicks * x;
			kf.wavePower     = lhs.wavePower * x;
			kf.waveStrength  = lhs.waveStrength * x;
			kf.worldScale    = lhs.worldScale * x;
			kf.colorTicks    = uint64_t(lhs.colorTicks * x);
			for(size_t i = 0; i < kf.colors.size(); ++i) {
				kf.colors[i] = lhs.colors[i] * x;
			}
			return kf;
		}
	};

	inline serializer &operator<<(serializer &s, const keyframe &kf) {
		return s << kf.ticks << kf.baseThreshold << kf.beatTicks << kf.beatPower
		         << kf.beatStrength << kf.waveTicks << kf.wavePower
		         << kf.waveStrength << kf.worldScale << kf.colorTicks
		         << kf.colors;
	}

	inline deserializer &operator>>(deserializer &s, keyframe &kf) {
		return s >> kf.ticks >> kf.baseThreshold >> kf.beatTicks >>
		       kf.beatPower >> kf.beatStrength >> kf.waveTicks >>
		       kf.wavePower >> kf.waveStrength >> kf.worldScale >>
		       kf.colorTicks >> kf.colors;
	}
} // namespace polar::support::level
