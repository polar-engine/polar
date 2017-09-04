#pragma once

#include <chrono>
#include <polar/core/types.h>

#define ENGINE_TICKS_PER_SECOND 10000

typedef std::chrono::duration<uint64_t, std::ratio<1, ENGINE_TICKS_PER_SECOND>> DeltaTicksBase;

class DeltaTicks {
private:
	DeltaTicksBase value;
public:
	using seconds_type = Decimal;

	DeltaTicks() {}
	DeltaTicks(const DeltaTicksBase &value) : value(value) {}
	DeltaTicks(const DeltaTicksBase::rep ticks) : value(ticks) {}

	inline DeltaTicksBase::rep Ticks() const {
		return value.count();
	}

	inline seconds_type Seconds() const {
		return value.count() / static_cast<seconds_type>(ENGINE_TICKS_PER_SECOND);
	}

	inline void SetSeconds(const seconds_type &seconds) {
		value = DeltaTicksBase(static_cast<DeltaTicksBase::rep>(seconds * ENGINE_TICKS_PER_SECOND));
	}

	inline bool operator<(const DeltaTicksBase &rhs) { return value < rhs; }
	inline bool operator<(const DeltaTicks &rhs) { return *this < rhs.value; }
	inline bool operator>(const DeltaTicksBase &rhs) { return value < rhs; }
	inline bool operator>(const DeltaTicks &rhs) { return *this < rhs.value; }
	inline bool operator<=(const DeltaTicksBase &rhs) { return !(*this > rhs); }
	inline bool operator<=(const DeltaTicks &rhs) { return !(*this > rhs.value); }
	inline bool operator>=(const DeltaTicksBase &rhs) { return !(*this < rhs); }
	inline bool operator>=(const DeltaTicks &rhs) { return !(*this < rhs.value); }

	inline DeltaTicks & operator+=(const DeltaTicksBase &rhs) {
		value += rhs;
		return *this;
	}
	inline DeltaTicks & operator+=(const DeltaTicks &rhs) { return *this += rhs.value; }
	inline friend DeltaTicks operator+(DeltaTicks lhs, const DeltaTicksBase &rhs) { return lhs += rhs; }
	inline friend DeltaTicks operator+(DeltaTicks lhs, const DeltaTicks &rhs) { return lhs += rhs; }

	inline DeltaTicks & operator-=(const DeltaTicksBase &rhs) {
		value -= rhs;
		return *this;
	}
	inline DeltaTicks & operator-=(const DeltaTicks &rhs) { return *this -= rhs.value; }
	inline friend DeltaTicks operator-(DeltaTicks lhs, const DeltaTicksBase &rhs) { return lhs -= rhs; }
	inline friend DeltaTicks operator-(DeltaTicks lhs, const DeltaTicks &rhs) { return lhs -= rhs; }
};
