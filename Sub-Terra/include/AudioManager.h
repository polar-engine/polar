#pragma once

#include <portaudio.h>
#include "System.h"
#include "Oscillator.h"
#include "AudioAsset.h"
#include "AssetManager.h"

int StreamCallback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *);

class AudioManager : public System {
private:
	PaStream *stream;
	Oscillator osc;
	AudioAsset music;
protected:
	virtual void Init() override {
		music = engine->GetSystem<AssetManager>().lock()->Get<AudioAsset>("nexus");
		Pa_StartStream(stream);
	}
public:
	const double sampleRate = 44100.0;
	const unsigned long framesPerBuffer = 256;
	size_t currentSample = 0;

	static bool IsSupported() { return true; }

	AudioManager(Polar *engine) : System(engine), osc(MkSineWaveShape(), SCIENTIFIC_A) {
		Pa_Initialize();
		Pa_OpenDefaultStream(&stream, 0, 2, paInt16, sampleRate, framesPerBuffer, StreamCallback, this);
	}

	~AudioManager() {
		Pa_CloseStream(stream);
		Pa_Terminate();
	}

	int Tick(int16_t *buffer, unsigned long frameCount) {
		for(unsigned long frame = 0; frame < frameCount; ++frame) {
			buffer[frame * 2 + 0] = music.samples[currentSample++];
			buffer[frame * 2 + 1] = music.samples[currentSample++];
			if(currentSample == music.samples.size()) { currentSample = 3565397 * 2; }
		}
		return paContinue;
	}
};
