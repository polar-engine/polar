#pragma once

#include <polar/asset/base.h>
#include <polar/support/level/keyframe.h>
#include <set>

namespace polar {
namespace asset {
	struct level : base {
		using keyframe = support::level::keyframe;

		std::set<keyframe> keyframes;
		uint64_t ticks;

		explicit level(std::set<keyframe> kfs = {}, uint64_t t = 0)
		    : keyframes(kfs), ticks(t) {}

		const keyframe &get_current() const;
		const keyframe &get_next() const;
		keyframe get_now() const;
	};

	template <> inline std::string name<level>() { return "level"; }

	inline serializer &operator<<(serializer &s, const level &level) {
		return s << level.keyframes;
	}

	inline deserializer &operator>>(deserializer &s, level &level) {
		return s >> level.keyframes;
	}
}
}
