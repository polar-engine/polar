#pragma once

#include "Serializer.h"
#include "Asset.h"

struct AudioAsset : Asset {
	bool stereo;
	uint32_t sampleRate;
	std::vector<int16_t> samples;
};

inline Serializer & operator<<(Serializer &s, AudioAsset asset) {
	return s << asset.stereo << asset.sampleRate << asset.samples;
}

inline Deserializer & operator>>(Deserializer &s, AudioAsset &asset) {
	return s >> asset.stereo >> asset.sampleRate >> asset.samples;
}

template<> inline std::string AssetName<AudioAsset>() { return "Audio"; }
