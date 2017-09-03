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
	uint64_t colorTicks;
	std::array<Point3, 3> colors;

	explicit Keyframe(uint64_t t = 0) : ticks(t) {}
	explicit Keyframe(uint64_t t, const Keyframe &kf) : Keyframe(kf) { ticks = t; }

	friend std::ostream & operator<<(std::ostream &s, const Keyframe &kf) {
		s << "ticks         = " << kf.ticks         << std::endl;
		s << "baseThreshold = " << kf.baseThreshold << std::endl;
		s << "beatTicks     = " << kf.beatTicks     << std::endl;
		s << "beatPower     = " << kf.beatPower     << std::endl;
		s << "beatStrength  = " << kf.beatStrength  << std::endl;
		s << "waveTicks     = " << kf.waveTicks     << std::endl;
		s << "wavePower     = " << kf.wavePower     << std::endl;
		s << "waveStrength  = " << kf.waveStrength  << std::endl;
		s << "colorTicks    = " << kf.colorTicks    << std::endl;
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
		kf.colorTicks    = lhs.colorTicks + rhs.colorTicks;
		for(size_t i = 0; i < kf.colors.size(); ++i) {
			kf.colors[i]  = lhs.colors[i]    + rhs.colors[i];
		}
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
		kf.colorTicks    = uint64_t(lhs.colorTicks * x);
		for(size_t i = 0; i < kf.colors.size(); ++i) {
			kf.colors[i]  = lhs.colors[i]    * x;
		}
		return kf;
	}
};

inline Serializer & operator<<(Serializer &s, const Keyframe &kf) {
	return s << kf.ticks << kf.baseThreshold << kf.beatTicks << kf.beatPower << kf.beatStrength << kf.waveTicks << kf.wavePower << kf.waveStrength << kf.worldScale << kf.colorTicks << kf.colors;
}

inline Deserializer & operator>>(Deserializer &s, Keyframe &kf) {
	return s >> kf.ticks >> kf.baseThreshold >> kf.beatTicks >> kf.beatPower >> kf.beatStrength >> kf.waveTicks >> kf.wavePower >> kf.waveStrength >> kf.worldScale >> kf.colorTicks >> kf.colors;
}

struct Level : Asset {
	std::set<Keyframe> keyframes;
	uint64_t ticks;

	explicit Level(std::set<Keyframe> kfs = {}, uint64_t t = 0) : keyframes(kfs), ticks(t) {}

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

		auto diff = Decimal(next.ticks - current.ticks);
		auto alpha = diff > 0 ? Decimal(ticks - current.ticks) / diff : 1;
		auto kf = next * alpha + current * (1 - alpha);
		kf.ticks = ticks;
		return kf;
	}
};

template<> inline std::string AssetName<Level>() { return "Level"; }

inline Serializer & operator<<(Serializer &s, const Level &level) {
	return s << level.keyframes;
}

inline Deserializer & operator>>(Deserializer &s, Level &level) {
	return s >> level.keyframes;
}
