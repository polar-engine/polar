#pragma once

#include <atomic>
#include <boost/array.hpp>
#include <boost/container/flat_map.hpp>
#include <portaudio.h>
#include "System.h"
#include "Oscillator.h"
#include "AudioAsset.h"
#include "AssetManager.h"

int StreamCallback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *);

struct LoopIn {
	size_t value;
};

struct LoopOut {
	size_t value;
};

class AudioSource : public Component {
protected:
	AudioAsset asset;
	bool looping;
	size_t loopIn;
	size_t loopOut;
	size_t currentSample = 0;
public:

	AudioSource(const AudioAsset &as, const bool looping = false)            : asset(as), looping(looping), loopIn(0),        loopOut(as.samples.size()) {}
	AudioSource(const AudioAsset &as, const LoopIn  &in)                     : asset(as), looping(true),    loopIn(in.value), loopOut(as.samples.size()) {}
	AudioSource(const AudioAsset &as, const LoopOut &out)                    : asset(as), looping(true),    loopIn(0),        loopOut(out.value) {}
	AudioSource(const AudioAsset &as, const LoopIn  &in, const LoopOut &out) : asset(as), looping(true),    loopIn(in.value), loopOut(out.value) {}

	bool Tick(int16_t &left, int16_t &right) {
		if(looping) {
			if(currentSample == loopOut || currentSample == asset.samples.size()) { currentSample = loopIn; }
		} else if(currentSample == asset.samples.size()) { return false; }

		if(asset.stereo) {
			left  += asset.samples[currentSample];
			right += asset.samples[currentSample + 1];
			currentSample += 2;
		} else {
			left  += asset.samples[currentSample];
			right += asset.samples[currentSample];
			++currentSample;
		}
		return true;
	}
};

enum class ChannelMessageType {
	Add,
	Remove
};

struct ChannelMessage {
	static ChannelMessage Add(IDType id, AudioSource *source) { return ChannelMessage{ChannelMessageType::Add, id, source}; }
	static ChannelMessage Remove(IDType id) { return ChannelMessage{ChannelMessageType::Remove, id}; }

	ChannelMessageType type;
	IDType id;
	AudioSource *source;
};

class AudioManager : public System {
private:
	static const int channelSize = 8;
	boost::array<ChannelMessage, channelSize> channel;
	std::atomic_int channelWaiting = 0;
	int channelIndexMain = 0;
	int channelIndexStream = 0;

	boost::container::flat_map<IDType, AudioSource> sources;

	PaStream *stream;
protected:
	virtual void Init() override {
		Pa_StartStream(stream);
	}

	void ComponentAdded(IDType id, const std::type_info *ti, std::weak_ptr<Component> ptr) override final {
		if(ti != &typeid(AudioSource)) { return; }
		auto source = static_cast<AudioSource *>(ptr.lock().get());

		channel[channelIndexMain] = ChannelMessage::Add(id, source);
		++channelWaiting;
		++channelIndexMain;
		if(channelIndexMain == channelSize) { channelIndexMain = 0; }
	}

	void ComponentRemoved(IDType id, const std::type_info *ti) override final {
		if(ti != &typeid(AudioSource)) { return; }

		channel[channelIndexMain] = ChannelMessage::Remove(id);
		++channelWaiting;
		++channelIndexMain;
		if(channelIndexMain == channelSize) { channelIndexMain = 0; }
	}
public:
	const double sampleRate = 44100.0;
	const unsigned long framesPerBuffer = 256;

	static bool IsSupported() { return true; }

	AudioManager(Polar *engine) : System(engine) {
		Pa_Initialize();
		Pa_OpenDefaultStream(&stream, 0, 2, paInt16, sampleRate, framesPerBuffer, StreamCallback, this);
	}

	~AudioManager() {
		Pa_CloseStream(stream);
		Pa_Terminate();
	}

	int Tick(int16_t *buffer, unsigned long frameCount) {
		while(channelWaiting > 0) {
			auto msg = channel[channelIndexStream];
			switch(msg.type) {
			case ChannelMessageType::Add:
				sources.emplace(msg.id, *msg.source);
				break;
			case ChannelMessageType::Remove:
				sources.erase(msg.id);
				break;
			}
			--channelWaiting;
			++channelIndexStream;
			if(channelIndexStream == channelSize) { channelIndexStream = 0; }
		}

		for(unsigned long frame = 0; frame < frameCount; ++frame) {
			auto &left = buffer[frame * 2];
			auto &right = buffer[frame * 2 + 1];
			left = 0;
			right = 0;
			for(auto &source : sources) {
				source.second.Tick(left, right);
			}
		}
		return paContinue;
	}
};
