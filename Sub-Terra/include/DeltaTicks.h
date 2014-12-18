#pragma once

class DeltaTicks {
private:
	DeltaTicksBase value;
public:
	typedef float seconds_type;

	DeltaTicks() {}
	DeltaTicks(const DeltaTicksBase &value) : value(value) {}
	DeltaTicks(const DeltaTicksBase::rep ticks) : value(ticks) {}

	inline DeltaTicksBase::rep Ticks() const {
		return value.count();
	}

	inline seconds_type Seconds() const {
		return value.count() / static_cast<seconds_type>(ENGINE_TICKS_PER_SECOND);
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
