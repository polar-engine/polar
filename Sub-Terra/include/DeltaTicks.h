#pragma once

class DeltaTicks {
private:
	DeltaTicksBase base;
public:
	typedef float seconds_type;

	DeltaTicks(const DeltaTicksBase &base) : base(base) {}
	DeltaTicks(const DeltaTicksBase::rep ticks) : base(ticks) {}

	inline DeltaTicksBase::rep Ticks() const {
		return base.count();
	}

	inline seconds_type Seconds() const {
		return base.count() / static_cast<seconds_type>(ENGINE_TICKS_PER_SECOND);
	}
};
