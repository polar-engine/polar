#pragma once

#include <polar/asset/base.h>

namespace polar { namespace asset {
	struct audio : base {
		bool stereo;
		uint32_t sampleRate;
		raw_vector<int16_t> samples;
	};

	inline serializer & operator<<(serializer &s, audio &asset) {
		return s << asset.stereo << asset.sampleRate << asset.samples;
	}

	inline deserializer & operator>>(deserializer &s, audio &asset) {
		return s >> asset.stereo >> asset.sampleRate >> asset.samples;
	}

	template<> inline std::string name<audio>() { return "audio"; }
} }
