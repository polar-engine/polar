#pragma once

class DeltaTicks {
private:
	DeltaTicksBase base;
public:
	DeltaTicks(const DeltaTicksBase &base) : base(base) {}
	DeltaTicks(const DeltaTicksBase::rep ticks) : base(ticks) {}

	inline DeltaTicksBase::rep Ticks() const {
		return base.count();
	}

	inline float Seconds() const {
		return base.count() / static_cast<float>(ENGINE_TICKS_PER_SECOND);
	}
};
