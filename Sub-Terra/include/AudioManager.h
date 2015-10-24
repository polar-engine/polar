#pragma once

#include <portaudio.h>
#include "System.h"
#include "Oscillator.h"

int StreamCallback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *);

class AudioManager : public System {
private:
	PaStream *stream;
protected:
	virtual void Init() override {
		Pa_StartStream(stream);
	}
public:
	Oscillator osc;
	Oscillator phaser;
	const double sampleRate = 44100.0;
	const unsigned long framesPerBuffer = 256;

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
			auto sample = osc.Tick(sampleRate);
			buffer[frame * 2 + 0] = sample;
			buffer[frame * 2 + 1] = sample;
		}
		return paContinue;
	}
};
