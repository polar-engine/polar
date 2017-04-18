#pragma once

#include <stdint.h>
#include <set>

struct Keyframe {
	uint64_t ticks;
	Decimal baseThreshold;
	Decimal beatTicks;
	Decimal beatPower;
	Decimal beatStrength;
	Decimal waveTicks;
	Decimal wavePower;
	Decimal waveStrength;
	Point3  worldScale;

	Keyframe(uint64_t t) : ticks(t) {}
	Keyframe(uint64_t t, const Keyframe &kf) : Keyframe(kf) { ticks = t; }

	friend std::ostream & operator<<(std::ostream &s, const Keyframe &kf) {
		s << "ticks         = " << kf.ticks         << std::endl;
		s << "baseThreshold = " << kf.baseThreshold << std::endl;
		s << "beatTicks     = " << kf.beatTicks     << std::endl;
		s << "beatPower     = " << kf.beatPower     << std::endl;
		s << "beatStrength  = " << kf.beatStrength  << std::endl;
		s << "waveTicks     = " << kf.waveTicks     << std::endl;
		s << "wavePower     = " << kf.wavePower     << std::endl;
		s << "waveStrength  = " << kf.waveStrength  << std::endl;
		return s;
	}

	friend bool operator<(const Keyframe &a, const Keyframe &b) {
		return a.ticks < b.ticks;
	}

	friend Keyframe operator+(const Keyframe &lhs, const Keyframe &rhs) {
		Keyframe kf(lhs.ticks);
		kf.baseThreshold = lhs.baseThreshold + rhs.baseThreshold;
		kf.beatTicks     = lhs.beatTicks     + rhs.beatTicks;
		kf.beatPower     = lhs.beatPower     + rhs.beatPower;
		kf.beatStrength  = lhs.beatStrength  + rhs.beatStrength;
		kf.waveTicks     = lhs.waveTicks     + rhs.waveTicks;
		kf.wavePower     = lhs.wavePower     + rhs.wavePower;
		kf.waveStrength  = lhs.waveStrength  + rhs.waveStrength;
		kf.worldScale    = lhs.worldScale    + rhs.worldScale;
		return kf;
	}

	friend Keyframe operator*(const Keyframe &lhs, const Decimal x) {
		Keyframe kf(lhs.ticks);
		kf.baseThreshold = lhs.baseThreshold * x;
		kf.beatTicks     = lhs.beatTicks     * x;
		kf.beatPower     = lhs.beatPower     * x;
		kf.beatStrength  = lhs.beatStrength  * x;
		kf.waveTicks     = lhs.waveTicks     * x;
		kf.wavePower     = lhs.wavePower     * x;
		kf.waveStrength  = lhs.waveStrength  * x;
		kf.worldScale    = lhs.worldScale    * x;
		return kf;
	}
};

struct Level {
	std::set<Keyframe> keyframes;
	uint64_t ticks;

	Level(std::set<Keyframe> kfs, uint64_t t = 0) : keyframes(kfs), ticks(t) {}

	const Keyframe & GetCurrent() const {
		auto it = keyframes.lower_bound(Keyframe(ticks));
		if(it != keyframes.cbegin()) { --it; }
		return *it;
	}

	const Keyframe & GetNext() const {
		auto it = keyframes.lower_bound(Keyframe(ticks));
		if(it == keyframes.cend()) { --it; }
		return *it;
	}

	Keyframe GetNow() const {
		auto &current = GetCurrent();
		auto &next = GetNext();

		if(next.ticks == current.ticks) { return current; }

		auto alpha = Decimal(ticks - current.ticks) / Decimal(next.ticks - current.ticks);
		auto kf = next * alpha + current * (1 - alpha);
		kf.ticks = ticks;
		return kf;
	}
};