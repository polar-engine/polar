#include <polar/asset/level.h>

namespace polar {
namespace asset {
	using keyframe = support::level::keyframe;

	const keyframe &level::get_current() const {
		auto it = keyframes.lower_bound(keyframe(ticks));
		if(it != keyframes.cbegin()) { --it; }
		return *it;
	}

	const keyframe &level::get_next() const {
		auto it = keyframes.lower_bound(keyframe(ticks));
		if(it == keyframes.cend()) { --it; }
		return *it;
	}

	keyframe level::get_now() const {
		auto &current = get_current();
		auto &next    = get_next();

		if(next.ticks == current.ticks) { return current; }

		auto diff  = Decimal(next.ticks - current.ticks);
		auto alpha = diff > 0 ? Decimal(ticks - current.ticks) / diff : 1;
		auto kf    = next * alpha + current * (1 - alpha);
		kf.ticks   = ticks;
		return kf;
	}
}
}
