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
	const double sampleRate = 44100.0;
	const unsigned long framesPerBuffer = 256;

	static bool IsSupported() { return true; }

	AudioManager(Polar *engine) : System(engine) {
		osc = Oscillator(MkSineWaveShape());

		Pa_Initialize();
		Pa_OpenDefaultStream(&stream, 0, 2, paInt16, sampleRate, framesPerBuffer, StreamCallback, this);
	}

	~AudioManager() {
		Pa_CloseStream(stream);
		Pa_Terminate();
	}
};
