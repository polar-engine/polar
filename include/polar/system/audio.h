#pragma once

#include <array>
#include <atomic>
#include <polar/component/audiosource.h>
#include <polar/support/audio/channel.h>
#include <polar/support/audio/oscillator.h>
#include <polar/system/base.h>
#include <portaudio.h>

namespace polar::system {
	int audio_stream_cb(const void *, void *, unsigned long,
	                    const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags,
	                    void *);

	class audio : public base {
		using audiosource = component::audiosource;
		using sourcetype  = support::audio::sourcetype;
		using messagetype = support::audio::channel::messagetype;
		using message_t   = support::audio::channel::message;

	  private:
		static const int channelSize = 8;
		std::array<message_t, channelSize> channel;
		std::atomic_int channelWaiting = {0};
		int channelIndexMain           = 0;
		int channelIndexStream         = 0;

		std::unordered_map<IDType, std::shared_ptr<audiosource>> sources;

		PaStream *stream;

	  protected:
		virtual void init() override { Pa_StartStream(stream); }

		void componentadded(IDType id, std::type_index ti,
		                    std::weak_ptr<component::base> ptr) override {
			if(ti != typeid(audiosource)) { return; }

			channel[channelIndexMain] = message_t::add(
			    id, std::static_pointer_cast<audiosource>(ptr.lock()));
			++channelWaiting;
			++channelIndexMain;
			if(channelIndexMain == channelSize) { channelIndexMain = 0; }
		}

		void componentremoved(IDType id, std::type_index ti) override {
			if(ti != typeid(audiosource)) { return; }

			channel[channelIndexMain] = message_t::remove(id);
			++channelWaiting;
			++channelIndexMain;
			if(channelIndexMain == channelSize) { channelIndexMain = 0; }
		}

	  public:
		const double sampleRate             = 44100.0;
		const unsigned long framesPerBuffer = 256;
		std::atomic<bool> muted;

		std::atomic<int> masterVolume = {100};
		std::array<std::atomic<int>, size_t(sourcetype::_size)> volumes;

		static bool supported() { return true; }

		audio(core::polar *engine) : base(engine), muted(false) {
			for(auto &vol : volumes) { vol = 100; }
			Pa_Initialize();
			Pa_OpenDefaultStream(&stream, 0, 2, paInt16, sampleRate,
			                     framesPerBuffer, audio_stream_cb, this);
		}

		~audio() {
			Pa_CloseStream(stream);
			Pa_Terminate();
		}

		int tick(int16_t *buffer, unsigned long frameCount) {
			while(channelWaiting > 0) {
				auto msg = channel[channelIndexStream];
				switch(msg.type) {
				case messagetype::add:
					sources.emplace(msg.id, msg.source);
					break;
				case messagetype::remove:
					sources.erase(msg.id);
					break;
				}
				--channelWaiting;
				++channelIndexStream;
				if(channelIndexStream == channelSize) {
					channelIndexStream = 0;
				}
			}

			for(unsigned long frame = 0; frame < frameCount; ++frame) {
				auto &left  = buffer[frame * 2];
				auto &right = buffer[frame * 2 + 1];
				left        = 0;
				right       = 0;

				for(auto &source : sources) {
					double volume =
					    volumes[static_cast<size_t>(source.second->type)] /
					    100.0;
					source.second->tick(sampleRate, volume, left, right);
				}

				if(muted) {
					left  = 0;
					right = 0;
				} else {
					left  *= masterVolume / 100.0;
					right *= masterVolume / 100.0;
				}
			}
			return paContinue;
		}
	};
} // namespace polar::system
