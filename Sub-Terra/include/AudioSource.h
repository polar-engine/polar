#pragma once

#include "Component.h"
#include "AudioAsset.h"

struct LoopIn {
	size_t value;
};

struct LoopOut {
	size_t value;
};

class AudioSource : public Component {
protected:
	std::shared_ptr<AudioAsset> asset;
	bool looping;
	size_t loopIn;
	size_t loopOut;
	size_t currentSample = 0;
public:
	AudioSource(std::shared_ptr<AudioAsset> as, const bool looping = false) : asset(as), looping(looping), loopIn(0), loopOut(as->samples.size()) {}
	AudioSource(std::shared_ptr<AudioAsset> as, const LoopIn  &in) : asset(as), looping(true), loopIn(in.value), loopOut(as->samples.size()) {}
	AudioSource(std::shared_ptr<AudioAsset> as, const LoopOut &out) : asset(as), looping(true), loopIn(0), loopOut(out.value) {}
	AudioSource(std::shared_ptr<AudioAsset> as, const LoopIn  &in, const LoopOut &out) : asset(as), looping(true), loopIn(in.value), loopOut(out.value) {}

	bool Tick(int16_t &left, int16_t &right) {
		if(looping) {
			if(currentSample == loopOut || currentSample == asset->samples.size()) { currentSample = loopIn; }
		} else if(currentSample == asset->samples.size()) { return false; }

		if(asset->stereo) {
			left += asset->samples[currentSample];
			right += asset->samples[currentSample + 1];
			currentSample += 2;
		} else {
			left += asset->samples[currentSample];
			right += asset->samples[currentSample];
			++currentSample;
		}
		return true;
	}
};
