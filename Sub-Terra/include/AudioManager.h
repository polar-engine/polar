#pragma once

#include <portaudio.h>
#include "System.h"

int StreamCallback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *);

class AudioManager : public System {
private:
	PaStream *stream;
protected:
	virtual void Init() override {

	}

	virtual void Update(DeltaTicks &) override {

	}
public:
	const double sampleRate = 44100.0;
	const unsigned long framesPerBuffer = 256;

	static bool IsSupported() { return true; }

	AudioManager(Polar *engine) : System(engine) {
		Pa_Initialize();
		Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sampleRate, framesPerBuffer, StreamCallback, this);
	}

	~AudioManager() {
		Pa_Terminate();
	}
};
