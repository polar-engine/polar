#pragma once

#include <atomic>
#include <boost/array.hpp>
#include <boost/container/flat_map.hpp>
#include <portaudio.h>
#include "System.h"
#include "Oscillator.h"
#include "AudioSource.h"

int StreamCallback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *);

enum class ChannelMessageType {
	Add,
	Remove
};

struct ChannelMessage {
	static ChannelMessage Add(IDType id, std::shared_ptr<AudioSource> source) { return ChannelMessage{ChannelMessageType::Add, id, source}; }
	static ChannelMessage Remove(IDType id) { return ChannelMessage{ChannelMessageType::Remove, id}; }

	ChannelMessageType type;
	IDType id;
	std::shared_ptr<AudioSource> source;
};

class AudioManager : public System {
private:
	static const int channelSize = 8;
	boost::array<ChannelMessage, channelSize> channel;
	std::atomic_int channelWaiting = {0};
	int channelIndexMain = 0;
	int channelIndexStream = 0;

	boost::container::flat_map<IDType, std::shared_ptr<AudioSource>> sources;

	PaStream *stream;
protected:
	virtual void Init() override {
		Pa_StartStream(stream);
	}

	void ComponentAdded(IDType id, const std::type_info *ti, std::weak_ptr<Component> ptr) override final {
		if(ti != &typeid(AudioSource)) { return; }

		channel[channelIndexMain] = ChannelMessage::Add(id, std::static_pointer_cast<AudioSource>(ptr.lock()));
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
				sources.emplace(msg.id, msg.source);
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
				source.second->Tick(left, right);
			}
		}
		return paContinue;
	}
};
