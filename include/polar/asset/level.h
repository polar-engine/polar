#pragma once

#include <set>
#include <polar/asset/base.h>
#include <polar/support/level/keyframe.h>

namespace polar { namespace asset {
	struct level : base {
		using keyframe = support::level::keyframe;

		std::set<keyframe> keyframes;
		uint64_t ticks;

		explicit level(std::set<keyframe> kfs = {}, uint64_t t = 0) : keyframes(kfs), ticks(t) {}

		const keyframe & get_current() const {
			auto it = keyframes.lower_bound(keyframe(ticks));
			if(it != keyframes.cbegin()) { --it; }
			return *it;
		}

		const keyframe & get_next() const {
			auto it = keyframes.lower_bound(keyframe(ticks));
			if(it == keyframes.cend()) { --it; }
			return *it;
		}

		keyframe get_now() const {
			auto &current = get_current();
			auto &next = get_next();

			if(next.ticks == current.ticks) { return current; }

			auto diff = Decimal(next.ticks - current.ticks);
			auto alpha = diff > 0 ? Decimal(ticks - current.ticks) / diff : 1;
			auto kf = next * alpha + current * (1 - alpha);
			kf.ticks = ticks;
			return kf;
		}
	};

	template<> inline std::string name<level>() { return "level"; }

	inline serializer & operator<<(serializer &s, const level &level) {
		return s << level.keyframes;
	}

	inline deserializer & operator>>(deserializer &s, level &level) {
		return s >> level.keyframes;
	}
} }
