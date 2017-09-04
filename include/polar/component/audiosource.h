#pragma once

#include <polar/component/base.h>
#include <polar/asset/audio.h>

struct LoopIn {
	size_t value;
};

struct LoopOut {
	size_t value;
};

enum class AudioSourceType : size_t {
	None,
	Music,
	Effect,
	_Size
};

class AudioSource : public Component {
protected:
	std::shared_ptr<AudioAsset> asset;
	bool looping;
	size_t loopIn;
	size_t loopOut;
	size_t currentSample = 0;
public:
	AudioSourceType type = AudioSourceType::None;

	AudioSource(std::shared_ptr<AudioAsset> as, AudioSourceType type, const bool looping = false)            : asset(as), type(type), looping(looping), loopIn(0),        loopOut(as->samples.size()) {}
	AudioSource(std::shared_ptr<AudioAsset> as, AudioSourceType type, const LoopIn  &in)                     : asset(as), type(type), looping(true),    loopIn(in.value), loopOut(as->samples.size()) {}
	AudioSource(std::shared_ptr<AudioAsset> as, AudioSourceType type, const LoopOut &out)                    : asset(as), type(type), looping(true),    loopIn(0),        loopOut(out.value) {}
	AudioSource(std::shared_ptr<AudioAsset> as, AudioSourceType type, const LoopIn  &in, const LoopOut &out) : asset(as), type(type), looping(true),    loopIn(in.value), loopOut(out.value) {}

	void Advance(double sampleRate, int delta) {
		currentSample += size_t(double(delta) * (44100.0 / sampleRate));
	}

	bool Tick(double sampleRate, double volume, int16_t &left, int16_t &right) {
		if(looping) {
			if(currentSample == loopOut || currentSample >= asset->samples.size()) { currentSample = loopIn; }
		} else if(currentSample >= asset->samples.size()) { return false; }

		if(asset->stereo) {
			left  += asset->samples[currentSample]     * volume;
			right += asset->samples[currentSample + 1] * volume;
			Advance(sampleRate, 2);
		} else {
			left  += asset->samples[currentSample] * volume;
			right += asset->samples[currentSample] * volume;
			Advance(sampleRate, 1);
		}
		return true;
	}
};
