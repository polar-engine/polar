#pragma once

#include <polar/asset/base.h>

namespace polar::asset {
	struct audio : base {
		bool stereo;
		uint32_t sampleRate;
		uint32_t loopPoint = 0;
		raw_vector<int16_t> samples;
	};

	inline serializer &operator<<(serializer &s, audio &asset) {
		return s << asset.stereo << asset.sampleRate << asset.loopPoint << asset.samples;
	}

	inline deserializer &operator>>(deserializer &s, audio &asset) {
		return s >> asset.stereo >> asset.sampleRate >> asset.loopPoint >> asset.samples;
	}

	template<> inline std::string name<audio>() { return "audio"; }
} // namespace polar::asset
